//
// Created by fotyev on 2016-10-19.
//


#include "gamestate.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"


void gamestate_t::draw()
{
    for(auto& e : entities)
    {
        e->draw();
    }
}


gamestate_t::gamestate_t()
{
}

void gamestate_t::update(tick_t tick)
{
    //LOGGER(debug, "updating...", tick);
    for(auto& e : entities)
    {
        e->update(tick);
    }

}

void gamestate_t::on_input(tick_input_t& input)
{
    for(auto& e : entities)
    {
        e->on_input(input);
    }
}

bool gamestate_t::remove(entity_t * ent_ptr)
{
    auto it = std::remove_if(entities.begin(), entities.end(), [ent_ptr](const entity_ptr& e) { return e.get() == ent_ptr; });
    if(it != entities.end())
    {
        assert(it == entities.end() - 1);
        entities.pop_back();
        return true;
    }
    return false;
}

#include <boost/serialization/vector.hpp>
#include <boost/serialization/unique_ptr.hpp>

template <typename Archive>
void gamestate_t::serialize(Archive& ar, const unsigned)
{
    // TODO: register all entity types
    /*< join "\n\t", map { "ar.template register_type<$_->{name}>();" } dispatch('entities');
       %*//*>*/
    //ar.template register_type<basic_entity>();
    ar & entities;
}

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>

void gamestate_t::operator = (gamestate_t& rhs)
{
    std::stringstream ss;

    using namespace boost::archive;

    {
        binary_oarchive oa(ss, no_header | no_codecvt | no_tracking);
        oa & rhs;
    }
    {
        binary_iarchive ia(ss, no_header | no_codecvt | no_tracking);
        ia & *this;
    }

    for(auto& ent : entities) // glorious hack
        ent->gamestate = this;
}

void gamestate_simulator::push(tick_input_t inp)
{
    if(inp.tick < simulated_new)
        newstate_invalidated = true;
    inputs.push(std::move(inp));
}



void gamestate_simulator::update()
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

    if(newstate_invalidated)
    {
        // copy oldstate to newstate
        newstate = oldstate;

        simulated_new = simulated_old;
        newstate_invalidated = false;
    }

    // simulate newstate
    auto next_input = inputs.lower_bound(simulated_new);
    while(simulated_new < newtick)
    {
        while(next_input != inputs.buf.end())
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

