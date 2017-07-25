//
// Created by fotyev on 2016-10-19.
//

#if 0
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


void gamestate_t::update(tick_t tick)
{
    //LOGGER(debug, "updating...", tick);
    for(auto& e : entities)
    {
        e->update(tick);
    }

}

void gamestate_t::on_input(const tick_input_t& input)
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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>

void gamestate_t::operator = (gamestate_t& rhs)
{
    std::stringstream ss;

    using namespace boost::archive;

    {
        binary_oarchive oa(ss, no_header | no_codecvt);
        oa & rhs.entities;
    }
    {
        binary_iarchive ia(ss, no_header | no_codecvt);
        ia & entities;
    }

    for(auto& ent : entities) // glorious hack
        ent->gamestate = this;
}



#endif
