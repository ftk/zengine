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

struct old_input_exc {tick_input_t input; tick_t curtick;};

// Gamestate requirements: update(tick_t), on_input(tick_input_t)
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
    tick_t get_oldtick() const { return simulated_old; } //
    void clear() { inputs.buf.clear(); simulated_old = 0; }

    template <typename... Args>
    gamestate_simulator(Args&&... args) : oldstate(std::forward<Args>(args)...)
    {
    }

    void on_input(tick_input_t ev)
    {
        inputs.push(std::move(ev));
    }

    void update(tick_t curtick)
    {
        auto oldtick = (curtick < lag) ? 0 : (curtick - lag);


        // simulate oldstate
        while(simulated_old < oldtick)
        {
            if(!inputs.buf.empty() && inputs.buf.front().tick < simulated_old)
            {
                LOGGER(error, "TOO old", inputs.buf.front().tick, simulated_old);
                auto exc = old_input_exc{std::move(inputs.buf.front()), simulated_old};
                inputs.pop_old(simulated_old); // ?
                throw exc;
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

    Gamestate& gamestate()
    {
        return oldstate;
    }
};

#include <mutex>
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <type_traits>

// GamestateSync requirements: the same as Gamestate + serialize + operator=(optional)
template <class GamestateSync>
class gamestate_simulator2 : public gamestate_simulator<GamestateSync>
{
    typedef gamestate_simulator<GamestateSync> Base;
protected:
    tick_t simulated_new = 0; // invariant: should always be simulated_old + lag
    bool newstate_invalidated = false;
    GamestateSync newstate;

    std::mutex inputs_mtx;


public:

    template <typename... Args>
    gamestate_simulator2(Args&&... args) : Base(args...), newstate(std::forward<Args>(args)...)
    {
    }

    void on_input(tick_input_t ev)
    {
        std::lock_guard<std::mutex> lock(inputs_mtx);
        if(ev.tick < simulated_new)
            newstate_invalidated = true;
        Base::on_input(std::move(ev));
    }


    void update(tick_t curtick)
    {
        Base::update(curtick);

        std::lock_guard<std::mutex> lock(inputs_mtx);
        if(newstate_invalidated)
        {
            // copy oldstate to newstate
            LOGGER(debug2, "invalidating newstate", curtick, simulated_new, "->", this->simulated_old);
            invalidate_new_state();
            //newstate = this->oldstate;
            newstate_invalidated = false;
        }

        // simulate newstate

        auto next_input = this->inputs.lower_bound(simulated_new);
        while(simulated_new < curtick)
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

    GamestateSync& gamestate()
    {
        return newstate;
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
        using namespace cereal;
        {
            BinaryOutputArchive ar(ss);
            ar(this->oldstate);
        }
        {
            BinaryInputArchive ar(ss);
            ar(newstate);
        }
        simulated_new = this->simulated_old;
    }

public:
    // serializable
    template <class Archive>
    void save(Archive& ar) const
    {
        ar(this->oldstate, this->simulated_old);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        ar(this->oldstate, this->simulated_old);
        newstate_invalidated = true;
    }
};


#endif //ZENGINE_GAMESTATE_HPP
