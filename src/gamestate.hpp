//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_GAMESTATE_HPP
#define ZENGINE_GAMESTATE_HPP

#include "playerinputs.hpp"

#include "entity.hpp"

#include <vector>
#include <memory>
#include "util/movable.hpp"
#include "util/log.hpp"

class gamestate_t /*: public entity_t*/
{
public:
    using entity_ptr = std::unique_ptr<entity_t>;
protected:
    std::vector<entity_ptr> entities;
    friend struct entity_t;
public:

    gamestate_t() {}

    void update(tick_t tick);
    //void update(players_tick_input_t& t);
    void on_input(const tick_input_t& input);

    void draw();


    template <class Entity, typename... Args>
    Entity * emplace(Args&&... args)
    {
        entities.emplace_back(std::make_unique<Entity>(std::forward<Args>(args)...));
        entities.back()->gamestate = this;
        return static_cast<Entity *>(entities.back().get());
    }
    void insert(entity_ptr ent)
    {
        entities.push_back(std::move(ent));
        entities.back()->gamestate = this;
    }

    bool remove(entity_t * ent_ptr);

public:

    void operator = (gamestate_t& rhs);
};

class old_input_exc {};

// Gamestate requirements: update, on_input, draw
template <class Gamestate = gamestate_t>
class gamestate_simulator
{
    NONCOPYABLE(gamestate_simulator)
public:
    tick_t lag = 1;
protected:
    tick_t simulated_old = 0;
    inputs_t inputs;

protected:
    Gamestate oldstate;

public:

    Gamestate& state() { return oldstate; }

    template <typename... Args>
    gamestate_simulator(Args&&... args) : oldstate(std::forward<Args>(args)...)
    {
        simulated_old = get_tick();
    }

    void push(tick_input_t ev)
    {
        inputs.push(std::move(ev));
    }

    void update()
    {
        auto newtick = get_tick();
        auto oldtick = newtick - lag;


        // simulate oldstate
        while(simulated_old < oldtick)
        {
            if(!inputs.buf.empty() && inputs.buf.front().tick < simulated_old)
            {
                LOGGER(error, "TOO old", inputs.buf.front().tick, simulated_old);
                inputs.pop_old(simulated_old);
                throw old_input_exc{};
                //
            }

            while(!inputs.buf.empty() && inputs.buf.front().tick == simulated_old)
            {
                oldstate.on_input(inputs.buf.front());
                inputs.buf.pop_front();
            }
            oldstate.update(simulated_old);
            simulated_old++;
        }

    }

    void draw()
    {
        oldstate.draw();//newstate.draw();
    }

};

// GamestateSync requirements: the same as Gamestate, operator=, TODO: serialize, deserialize
template <class GamestateSync = gamestate_t>
class gamestate_simulator2 : public gamestate_simulator<GamestateSync>
{
    typedef gamestate_simulator<GamestateSync> Base;
protected:
    tick_t simulated_new = 0; // invariant: should always be simulated_old + lag
    volatile bool newstate_invalidated = true;
    GamestateSync newstate;


public:

    template <typename... Args>
    gamestate_simulator2(Args&&... args) : Base(args...), newstate(std::forward<Args>(args)...)
    {
        simulated_new = get_tick();
    }

    void push(tick_input_t ev) override
    {
        if(ev.tick < simulated_new)
            newstate_invalidated = true;
        Base::push(std::move(ev));
    }


    void update() override
    {
        Base::update();

        if(newstate_invalidated)
        {
            // copy oldstate to newstate
            newstate = this->oldstate;

            simulated_new = this->simulated_old;
            newstate_invalidated = false;
        }

        // simulate newstate
        auto newtick = get_tick();

        auto next_input = this->inputs.lower_bound(simulated_new);
        while(simulated_new < newtick)
        {
            while(next_input != this->inputs.buf.end())
            {
                assert(next_input->tick >= simulated_new);
                if(next_input->tick != simulated_new)
                    break;
                newstate.on_input(*next_input);
                ++next_input;
            }
            newstate.update(simulated_new);
            simulated_new++;
        }
    }

};


#endif //ZENGINE_GAMESTATE_HPP
