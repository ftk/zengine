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

#include "collision.hpp"


class paddle
{
public:
    qvm::vec2 pos;
    SERIALIZABLE((pos))

    paddle(qvm::vec2 pos) : pos(std::move(pos))
    {
    }
    paddle() {}

    qvm::vec2 size {0.05, 0.10};
    col::box bounding_box() const
    {
        return col::box{pos + size/2, size};
    }

    void draw()
    {
        auto& texture = g_app->textures->get_from_file("resources/paddle.png");
        g_app->window->render.copy2(texture, pos, size);
    }
    void on_input(const tick_input_t& input)
    {
        boost::apply_visitor(*this, input.event);
    }

    //=- register_event(name=>'movement', params=>[[qw(int32_t x)], [qw(int32_t y)]]);
    void operator()(event::movement m)
    {
        using namespace qvm;
        X(pos) = clamp(X(pos) + m.x / 1000.f, -1.f, 1.f);
        Y(pos) = clamp(Y(pos) + m.y / 1000.f, -1.f, 1.f);
    }


    template <typename T>
    void operator()(const T&) {}

};

class ball
{
public:
    qvm::vec2 pos, vel;
    qvm::vec2 size {0.01, 0.01};
    SERIALIZABLE((pos)(vel))

    ball(qvm::vec2 pos, qvm::vec2 vel) : pos(std::move(pos)), vel(std::move(vel)) {
    }

    col::box bounding_box() const
    {
        return col::box{pos + size/2, size};
    }

    void collide(const paddle& obj)
    {
        using namespace qvm;
        try
        {
            auto dir = normalized(pos - obj.bounding_box().center);
            vel = dir / 500;
        } catch(qvm::error) {}
    }

public:

    void update(tick_t tick)
    {
        pos += vel;
        // collision
    }

    void draw()
    {
        auto& texture = g_app->textures->get_from_file("resources/ball.png");
        g_app->window->render.copy2(texture, pos, size);
    }

};

class pongstate
{
public:
    ball b { {0,0}, {0.01, 0}};
    std::vector<paddle> p;

    std::vector<net_node_id> players;

    pongstate()
    {
    }

    void update(tick_t tick)
    {
        b.update(tick);

        for(auto& pp : p) if(col::simple_collider::test(b.bounding_box(), pp.bounding_box())) b.collide(pp);
    }
    void draw()
    {
        b.draw();
        for(unsigned int i = 0; i < p.size(); i++)
            p[i].draw();
    }
    void on_input(const tick_input_t& input)
    {

        for(unsigned int i = 0; i < p.size(); i++)
            if(input.player == players[i])
                p[i].on_input(input);
        if(input.event.which() == event::player_join::index)
        {
            // new player
            p.push_back(paddle{qvm::vec2{0.,0.}});
            players.push_back(input.player);
        }
    }

    SERIALIZABLE((b)(p)(players))

    /*void operator=(const pongstate& rhs)
    {
        b = rhs.b; p = rhs.p; players = ;
    }*/
};

/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(join host stop)]); >*/
class controller : public basic_module
{
    multiplayer_game<gamestate_simulator2<pongstate>> game {g_app->netgame.get()};

    event::movement mov;
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
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_S:
                        game.push_event(event::movement{0,-10}); return false;
                    case SDL_SCANCODE_W:
                        game.push_event(event::movement{0,10}); return false;
                    case SDL_SCANCODE_A:
                        game.push_event(event::movement{-10,0}); return false;
                    case SDL_SCANCODE_D:
                        game.push_event(event::movement{10,0}); return false;
                    default:
                        break;
                }break;
            case SDL_MOUSEMOTION:
            {
                mov.x += ev.motion.xrel;
                mov.y -= ev.motion.yrel;
                return false;
            }
        }

        return true;
    }

    void draw() override
    {
        game.sim.draw();
        if(mov.x || mov.y)
        {
            game.push_event(mov);
            mov.x = mov.y = 0;
        }
        game.update();
    }

};
#endif //ZENGINE_CONTROLLER_HPP
