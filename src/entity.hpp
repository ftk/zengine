//
// Created by fotyev on 2017-02-02.
//

#ifndef ZENGINE_ENTITY_HPP
#define ZENGINE_ENTITY_HPP


#include "playerinputs.hpp"


struct entity_t
{

    // draws entity, may be called from other thread
    virtual void draw() {};

    // called exactly once per tick
    virtual void update(tick_t tick) {};

    // called always before update()
    virtual void on_input(const tick_input_t&) {};

    // serialize (?)

    virtual ~entity_t() = default;
};

#endif //ZENGINE_ENTITY_HPP
