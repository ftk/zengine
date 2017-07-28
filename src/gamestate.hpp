//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_GAMESTATE_HPP
#define ZENGINE_GAMESTATE_HPP

#include "playerinputs.hpp"


#include <vector>
#include <memory>
#include "util/movable.hpp"
#include "util/log.hpp"

class old_input_exc {};

// Gamestate requirements: update, on_input, draw
template <class Gamestate>
class gamestate_simulator
{
    NONCOPYABLE(gamestate_simulator)
public:
    tick_t lag = 20;
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

#include <mutex>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <type_traits>

// GamestateSync requirements: the same as Gamestate + serialize + operator=(optional)
template <class GamestateSync>
class gamestate_simulator2 : public gamestate_simulator<GamestateSync>
{
    typedef gamestate_simulator<GamestateSync> Base;
protected:
    tick_t simulated_new = 0; // invariant: should always be simulated_old + lag
    bool newstate_invalidated = true;
    GamestateSync newstate;

    std::mutex inputs_mtx;


public:

    template <typename... Args>
    gamestate_simulator2(Args&&... args) : Base(args...), newstate(std::forward<Args>(args)...)
    {
        simulated_new = get_tick();
    }

    void push(tick_input_t ev)
    {
        std::lock_guard<std::mutex> lock(inputs_mtx);
        if(ev.tick < simulated_new)
            newstate_invalidated = true;
        Base::push(std::move(ev));
    }


    void update()
    {
        Base::update();

        std::lock_guard<std::mutex> lock(inputs_mtx);
        if(newstate_invalidated)
        {
            // copy oldstate to newstate
            invalidate_new_state();
            //newstate = this->oldstate;
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

    void draw()
    {
        newstate.draw();
    }

private:
    void invalidate_new_state() { invalidate_new_state_impl(std::is_copy_assignable<GamestateSync>{}); }
    // GamestateSync has operator=
    void invalidate_new_state_impl(std::true_type)
    {
        newstate = this->oldstate;
        simulated_new = this->simulated_old;
    }

    // or else just serialize and deserialize it
    void invalidate_new_state_impl(std::false_type)
    {
        std::stringstream ss;
        using namespace boost::archive;
        {
            binary_oarchive oa(ss, no_header | no_codecvt);
            oa & this->oldstate;
        }
        {
            binary_iarchive ia(ss, no_header | no_codecvt);
            ia & newstate;
        }
        simulated_new = this->simulated_old;
    }

public:
    std::string get_state() const
    {
        std::ostringstream ss;
        using namespace boost::archive;
        binary_oarchive oa(ss, no_header | no_codecvt);
        oa & this->oldstate;
        oa & this->simulated_old;
        return ss.str();
    }
    void set_state(string_view state)
    {
        std::istringstream ss(state);
        using namespace boost::archive;
        binary_oarchive ia(ss, no_header | no_codecvt);
        ia & this->oldstate;
        ia & this->simulated_old;

        invalidate_new_state();
    }
};


#endif //ZENGINE_GAMESTATE_HPP
