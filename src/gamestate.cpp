//
// Created by fotyev on 2016-10-19.
//


#include "gamestate.hpp"
#include "util/log.hpp"

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

void gamestate_simulator::push(tick_input_t inp)
{
    inputs.push(std::move(inp));
}



void gamestate_simulator::update()
{
    auto tick = get_tick() - lag;

    while(simulated < tick)
    {
        if(!inputs.buf.empty() && inputs.buf.front().tick < simulated)
        {
            LOGGER(error, "TOO old", inputs.buf.front().tick, simulated);
            inputs.pop_old(simulated);
            throw old_input_exc{};
            //
        }

        while(!inputs.buf.empty() && inputs.buf.front().tick == simulated)
        {
            oldstate.on_input(inputs.buf.front());
            inputs.buf.pop_front();
        }
        oldstate.update(simulated);
        simulated++;
    }
}

