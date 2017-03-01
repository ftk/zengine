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
#include "gamestate.hpp"

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

    static constexpr auto size = qvm::vec2{0.05, 0.10};
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
        X(pos) += m.x / 100.f;
        Y(pos) += m.y / 100.f;
    }


    template <typename T>
    void operator()(const T&) {}

};

class ball : public entity_t, public col::object
{
public:
    qvm::vec2 pos, vel;
    static constexpr qvm::vec2 size {0.01, 0.01};

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



/*< register_module(name=>'gamecontroller', class=>'gamecontroller', scriptexport=>[qw(startgame)]); >*/
class gamecontroller : public basic_module
{
    net_node_id remote = 0;
    net_node_id local;
    bool started = false;

    gamestate_simulator sim;
public:
    gamecontroller()
    {
        local = g_app->netgame->id();
        g_app->netgame->set_event_handler([this](const tick_input_t& input) {
            boost::apply_visitor([this, &input](const auto& event) -> void { this->on_netevent(input, event);}, input.event);
        });
    }

    void startgame(net_node_id remote)
    {
        if(started)
            return;
        this->remote = remote;
        //=- register_event(name=>'game_start');
        LOGGER(info, "game start sent to", remote);
        g_app->netgame->send_event(remote, event::game_start{});

    }

    void start()
    {
        LOGGER(info, "game started!!!", remote);
        sim.simulated = get_tick();

        assert(remote);

        started = true;
        sim.state().emplace<paddle>(qvm::vec2{-0.5,0}, std::min(local, remote));
        sim.state().emplace<paddle>(qvm::vec2{0.5,0}, std::max(local, remote));
        sim.state().emplace<ball>(qvm::vec2{0,0}, qvm::vec2{0.005,0});
        //sim.state().insert(new paddle{qvm::vec2{0.5,0}, std::max(local, remote)});
        //sim.state().insert(new ball{qvm::vec2{0,0}, qvm::vec2{0.01,0}});
        //g_app->modules->
    }

    void on_netevent(const tick_input_t& input, event::game_start)
    {
        if(started)
            return;
        //=- register_event(name=>'game_start_ack');
        remote = input.player;
        LOGGER(info, "game start received from", remote);
        g_app->netgame->send_event(remote, event::game_start_ack{});

        start();

    }
    void on_netevent(const tick_input_t& input, event::game_start_ack)
    {
        if(input.player != remote)
            return;
        LOGGER(info, "game start acknowledged");
        start();
    }

    template<typename T>
    void on_netevent(const tick_input_t& input, T event)
    {
        if(started)
        {
            sim.push(input);
        }
    }

    bool on_event(const SDL_Event& ev) override
    {
        if(!started)
            return true;
        switch(ev.type)
        {
            case SDL_KEYDOWN:
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_S:
                        push_local(event::movement{0,-1}); return false;
                    case SDL_SCANCODE_W:
                        push_local(event::movement{0,1}); return false;
                    case SDL_SCANCODE_A:
                        push_local(event::movement{-1,0}); return false;
                    case SDL_SCANCODE_D:
                        push_local(event::movement{1,0}); return false;
                    default:
                        break;
                }break;
            case SDL_MOUSEMOTION:
            {
                push_local(event::movement{ev.motion.xrel/6, -ev.motion.yrel/6});
                return false;
            }
        }

        return true;
    }

    void draw() override
    {
        if(started)
        {
            sim.update();
            sim.draw();
        }
    }

    void push_local(event_t event)
    {
        auto input = tick_input_t{g_app->netgame->id(), get_tick(), event};
        g_app->netgame->send_event(remote, event);
        sim.push(std::move(input));
    }

};
#endif //ZENGINE_CONTROLLER_HPP
