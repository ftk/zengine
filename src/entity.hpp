//
// Created by fotyev on 2017-02-02.
//

#ifndef ZENGINE_ENTITY_HPP
#define ZENGINE_ENTITY_HPP


#include "playerinputs.hpp"
#include "util/assert.hpp"

class gamestate_t;

struct entity_t
{
protected:
    gamestate_t * gamestate;
    friend class gamestate_t;
public:
    gamestate_t& state() const noexcept { assume(gamestate); return *gamestate; }

    // draws entity, may be called from other thread
    virtual void draw() {};

    // called exactly once per tick
    virtual void update(tick_t tick) {};

    // called always before update()
    virtual void on_input(const tick_input_t&) {};

    // serialize : do NOT serialize gamestate, gamestate_t should handle this
    SERIALIZABLE()

    virtual ~entity_t() = default;
};

#endif //ZENGINE_ENTITY_HPP
