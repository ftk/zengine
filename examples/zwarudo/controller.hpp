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
    static const unsigned gw = 40, gh = 40;
    static const qvm::vec2 size { 2.f / gw, 2.f / gh };

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

    // pos is center
    qvm::vec2 check_collision(qvm::vec2 pos, qvm::vec2 size) // returns vector of counter force is object is stuck somewhere, or 0,0
    {
        using namespace qvm;
        vec2 f {0,0 };
        unsigned c = 0;
        if(solid(pos)) // bottom left corner stuck
        {
            f += vec2{1, 1};
            c++;
        }
        if(solid(pos + X0(size))) // bottom right
        {
            f += vec2{-1, 1};
            c++;
        }
        if(solid(pos + _0Y(size))) // up left
        {
            f += vec2{1, -1};
            c++;
        }
        if(solid(pos + size)) // up right
        {
            f += vec2{-1, -1};
            c++;
        }

        if(!c)
            return vec2{0, 0};

        if(f == vec2{0, 0} && c)
            f = -pos; // push to 0,0 if all 4 corners are stuck
        return normalized(f) / 500;
    }

    SERIALIZABLE((w))
};

class player
{
    qvm::vec2 pos{0,0}, vel{0,-0.01};

    int dir = 0;

    static const qvm::vec2 size{0.05, 0.05};
public:

    void draw()
    {
        auto& texture = g_app->textures->get_from_file("resources/zee.png");
        g_app->window->render.copy2(texture, pos, size);
    }

    void update(tick_t tick, world& w)
    {
        using namespace qvm;
        pos += vel;

        // friction...
        vel *= 0.96;

        // gravity
        vel += vec2{0,-0.0005};


        // stop
        vec2 force = w.check_collision(pos, size);
        if(force != vec2{0,0})
            vel *= 0.5;
        vel += force;
        if(dir && abs(X(vel)) < 0.004)
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

    SERIALIZABLE((pos)(vel)(dir))
};


class zwstate
{
public:
    std::unordered_map<net_node_id, player> players;
    world w;

    zwstate()
    {
    }

    void update(tick_t tick)
    {
        for(auto& p : players)
            p.second.update(tick, w);
    }

    void draw()
    {
        // draw world
        w.draw();
        for(auto& p : players)
            p.second.draw();
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
        }
        else if(input.event.which() == event::player_leave::index)
        {
            players.erase(input.player);
        }
        //=- register_event(name=>'click', params=>[[qw(float x)], [qw(float y)]]);
        else if(const event::click * ev = boost::get<event::click>(&input.event))
        {
            w.flip({ev->x, ev->y});
        }
    }

    SERIALIZABLE((players)(w))
};

/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(join host stop send_event playing)]); >*/
class controller : public basic_module
{
    multiplayer_game<gamestate_simulator2<zwstate>> game {g_app->netgame.get()};

    decltype(game.state) prevstate = game.state;

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

    void draw() override
    {
        game.sim.draw();
        game.update();

        auto newstate = game.state;

        if(prevstate != newstate)
        {
            switch(newstate)
            {
                case decltype(game)::CLIENT:
                    {
                        //=- register_callback('void ()', 'on_connect');
                        g_app->script->on_connect();
                    }break;
                case decltype(game)::STOPPED:
                    {
                        //=- register_callback('void ()', 'on_disconnect');
                        g_app->script->on_disconnect();
                    }break;
                case decltype(game)::HOST:
                    {
                        //=- register_callback('void ()', 'on_host');
                        g_app->script->on_host();
                    }break;
                default: break;
            }
        }

        prevstate = newstate;
    }

    void send_event(event_t event)
    {
        if(game.playing())
            game.push_event(std::move(event));
    }

    bool playing() const { return game.playing(); }

};
#endif //ZENGINE_CONTROLLER_HPP
