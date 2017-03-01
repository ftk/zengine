//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_BASIC_MODULE_HPP
#define ZENGINE_BASIC_MODULE_HPP

#include <SDL.h>
#include "util/movable.hpp"

struct basic_module
{
    NONCOPYABLE(basic_module)
    basic_module() = default;

    // if on_event returns false the event will not be passed to next handlers
    virtual bool on_event(const SDL_Event& ev) { return true; }

    // called each frame
    virtual void draw() {}

    virtual ~basic_module() = default;

};

#endif //ZENGINE_BASIC_MODULE_HPP
