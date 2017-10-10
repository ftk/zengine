//
// Created by fotyev on 2017-02-15.
//

#ifndef ZENGINE_CONTROLLER_HPP
#define ZENGINE_CONTROLLER_HPP

#include "main.hpp"
#include "components/collections.hpp"
#include "components/window.hpp"
#include "components/netgame.hpp"
#include "components/script.hpp"
#include "gamestate.hpp"
#include "playercontroller.hpp"

#include "util/log.hpp"
#include "util/assert.hpp"

#include <unordered_map>
#include <bitset>
#include "modules/basic_module.hpp"

#include "opengl/math.hpp"

#include <memory>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/memory.hpp>

class world
{
    static constexpr unsigned gw = 40, gh = 40;
    static constexpr qvm::vec2 size { 2.f / gw, 2.f / gh };

    std::bitset<gw * gh> w;
public:
    world()
    {

        for(int i = 0; i < gw; i++)
            w[i * gh + 1] = true;
    }

    void draw()
    {
        auto& texture = g_app->textures->get_from_file("resources/block.png");

        using namespace qvm;

        for(int i = 0; i < gw; i++)
        {
            for(int j = 0; j < gh; j++)
            {
                if(w[i * gh + j])
                {
                    vec2 pos = {-1, -1};
                    X(pos) += X(size) * i;
                    Y(pos) += Y(size) * j;

                    g_app->window->render.copy2(texture, pos, size);
                }
            }

        }

    }

    bool solid(qvm::vec2 pos) const
    {
        using namespace qvm;
        pos += vec2{1, 1};
        int bx = X(pos) / X(size);
        int by = Y(pos) / Y(size);
        if(bx < 0 || bx >= gw || by < 0 || by >= gh) return true;
        return w[bx * gh + by];
    }

    void flip(qvm::vec2 pos)
    {
        using namespace qvm;
        pos += vec2{1, 1};
        int bx = X(pos) / X(size);
        int by = Y(pos) / Y(size);
        if(bx < 0 || bx >= gw || by < 0 || by >= gh) return;
        w[bx * gh + by] = !w[bx * gh + by];
    }

    SERIALIZABLE((w))
};

class player
{
    qvm::vec2 pos{0,0}, vel{0,-0.01};

    int dir = 0;

    static constexpr qvm::vec2 size{0.05, 0.05};
public:

    std::shared_ptr<world> w;

    void draw()
    {
        auto& texture = g_app->textures->get_from_file("resources/zee.png");
        g_app->window->render.copy2(texture, pos, size);
    }

    void update(tick_t tick)
    {
        using namespace qvm;
        pos += vel;
        // gravity

        // friction...
        vel *= 0.95;

        vel += vec2{0,-0.0003};


        // stop
        if(w->solid(pos))
            vel = vec2{0,0};

        if(dir && abs(X(vel)) < 0.003)
            X(vel) += dir / 400.f;

    }

    //=- register_event(name=>'movement', params=>[[qw(int32_t x)], [qw(int32_t y)], [qw(uint8_t down)]]);
    void operator()(event::movement m)
    {
        if(m.y == 0)
        {
            dir = m.down ? m.x : 0;
        }
        else if(m.down)
        {
            qvm::Y(vel) += m.y / 70.f;
        }
    }

    template <typename T>
    void operator()(T event) {}

    SERIALIZABLE((pos)(vel)(w))
};


class zwstate
{
public:
    std::unordered_map<net_node_id, player> players;
    std::shared_ptr<world> w = std::make_shared<world>();


    zwstate()
    {
    }

    void update(tick_t tick)
    {
        for(auto& p : players)
            p.second.update(tick);
    }

    void draw()
    {
        for(auto& p : players)
            p.second.draw();
        // draw world
        w->draw();
    }

    void on_input(const tick_input_t& input)
    {
        for(auto& p : players)
            if(input.player == p.first)
                boost::apply_visitor(p.second, input.event);

        if(input.event.which() == event::player_join::index)
        {
            // new player
            players.insert({input.player, player{}});
            players[input.player].w = w;
        }
        else if(input.event.which() == event::player_leave::index)
        {
            players.erase(input.player);
        }
        else if(const event::click * ev = boost::get<event::click>(&input.event))
        {
            w->flip({ev->x, ev->y});
        }
    }

    SERIALIZABLE((players)(w))

    void operator=(const zwstate& rhs)
    {
        *w = *rhs.w; players = rhs.players;
    }
};

/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(join host stop)]); >*/
class controller : public basic_module
{
    multiplayer_game<gamestate_simulator2<zwstate>> game {g_app->netgame.get()};

public:
    void join(net_node_id remote)
    {
        LOGGER(info, "sending join to", remote);
        game.join(remote);
    }

    void host()
    {
        LOGGER(info, "starting game");
        game.host();
    }

    void stop()
    {
        game.stop();
    }

    bool on_event(const SDL_Event& ev) override
    {
        switch(ev.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                if(ev.key.repeat)
                    break;
                bool down = (ev.key.type == SDL_KEYDOWN);
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_S:
                        game.push_event(event::movement{0, -1, down});
                        return false;
                    case SDL_SCANCODE_W:
                        game.push_event(event::movement{0, 1, down});
                        return false;
                    case SDL_SCANCODE_A:
                        game.push_event(event::movement{-1, 0, down});
                        return false;
                    case SDL_SCANCODE_D:
                        game.push_event(event::movement{1, 0, down});
                        return false;
                    default:
                        break;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                if(ev.button.state == SDL_PRESSED)
                {
                    auto pos = g_app->window->to_gl_coords({ev.button.x, ev.button.y});
                    //=- register_event(name=>'click', params=>[[qw(float x)], [qw(float y)]]);
                    game.push_event(event::click{qvm::X(pos), qvm::Y(pos)});
                }
                break;
            }
        }

        return true;
    }

    void draw() override
    {
        game.sim.draw();
        game.update();
    }

};
#endif //ZENGINE_CONTROLLER_HPP
