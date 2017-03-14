//
// Created by fotyev on 2017-02-15.
//

#ifndef ZENGINE_CONTROLLER_HPP
#define ZENGINE_CONTROLLER_HPP

#include "entity.hpp"

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


static col::simple_collider collider;

class paddle : public entity_t, public col::object
{
public:
    qvm::vec2 pos;
    net_node_id player;

    paddle(qvm::vec2 pos, net_node_id player) : pos(std::move(pos)), player(std::move(player)) {
        collider.objs.push_back(this);
    }

    qvm::vec2 size {0.05, 0.10};
    col::box bounding_box() const override
    {
        return col::box{pos + size/2, size};
    }

    void draw() override
    {
        auto& texture = g_app->textures->get_from_file("resources/paddle.png");
        g_app->window->render.set4dpos();
        g_app->window->render.copy(pos, pos + size, nullopt, texture);
    }
    void on_input(const tick_input_t& input) override
    {
        if(input.player == player)
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

class ball : public entity_t, public col::object
{
public:
    qvm::vec2 pos, vel;
    qvm::vec2 size {0.01, 0.01};

    ball(qvm::vec2 pos, qvm::vec2 vel) : pos(std::move(pos)), vel(std::move(vel)) {
        collider.objs.push_back(this);
    }

    col::box bounding_box() const override
    {
        return col::box{pos + size/2, size};
    }

    void collide(col::object * obj) override
    {
        using namespace qvm;
        try
        {
            auto dir = normalized(pos - obj->bounding_box().center);
            vel = dir / 500;
        } catch(qvm::error) {}
    }

public:

    void update(tick_t tick) override
    {
        pos += vel;
        // collision
        collider.test_object(this);
    }

    void draw() override
    {
        auto& texture = g_app->textures->get_from_file("resources/ball.png");
        g_app->window->render.set4dpos();
        g_app->window->render.copy(pos, pos + size, nullopt, texture);
    }

};


class gamecontroller
{
    event::movement mov{0,0};
    gamestate_simulator sim;
    net_node_id local, remote;
public:
    gamecontroller(net_node_id local, net_node_id remote) : local(local), remote(remote)
    {
        LOGGER(info, "game started!!!", remote);
        assert(remote);
        sim.start();


        sim.state().emplace<paddle>(qvm::vec2{-0.5,0}, std::min(local, remote));
        sim.state().emplace<paddle>(qvm::vec2{0.5,0}, std::max(local, remote));
        sim.state().emplace<ball>(qvm::vec2{0,0}, qvm::vec2{0.005,0});
        //sim.state().insert(new paddle{qvm::vec2{0.5,0}, std::max(local, remote)});
        //sim.state().insert(new ball{qvm::vec2{0,0}, qvm::vec2{0.01,0}});

        //=- register_callback('void (uint64_t)', 'on_game_start');
        g_app->script->on_game_start(remote);
    }



    bool on_event(const SDL_Event& ev)
    {
        switch(ev.type)
        {
            case SDL_KEYDOWN:
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_S:
                        push_local(event::movement{0,-10}); return false;
                    case SDL_SCANCODE_W:
                        push_local(event::movement{0,10}); return false;
                    case SDL_SCANCODE_A:
                        push_local(event::movement{-10,0}); return false;
                    case SDL_SCANCODE_D:
                        push_local(event::movement{10,0}); return false;
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

    void draw()
    {
        try
        {
            sim.update();
        }
        catch(old_input_exc)
        {
            push_local(event::disconnect{});
//            throw stop_game{};
        }
        sim.draw();

        if(mov.x || mov.y)
        {
            push_local(mov);
            mov.x = mov.y = 0;
        }
    }

    void push_local(event_t event)
    {
        auto input = tick_input_t{g_app->netgame->id(), get_tick(), event};
        g_app->netgame->send_event(remote, event);
        sim.push(std::move(input));
    }

    template<typename T>
    void on_netevent(const tick_input_t& input, T event)
    {
        sim.push(input);
    }

    ~gamecontroller()
    {
        LOGGER(info, "player disconnected :(");
        //=- register_callback('void ()', 'on_disconnect');
        g_app->script->on_disconnect();
    }
};


/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(startgame)]); >*/
class controller : public basic_module
{
    two_player_game<gamecontroller> game {g_app->netgame.get()};
public:
    void startgame(net_node_id remote)
    {
        game.send_request(remote);
    }

    bool on_event(const SDL_Event& ev) override
    {
        if(game.gamecontroller)
            return game.gamecontroller->on_event(ev);
        return true;
    }

    void draw() override
    {
        if(game.gamecontroller)
            game.gamecontroller->draw();
    }

};
#endif //ZENGINE_CONTROLLER_HPP
