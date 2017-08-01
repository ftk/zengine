//
// Created by fotyev on 2017-03-01.
//

#ifndef ZENGINE_COLLISION_HPP
#define ZENGINE_COLLISION_HPP

#include <vector>

#include "opengl/math.hpp"

namespace col {

using namespace qvm;

struct box
{
    vec2 center, size;

    float& x() { return X(center); }
    float& y() { return Y(center); }
    float& width() { return X(size); }
    float& height() { return Y(size); }
};

class simple_collider
{
public:
    static bool test(box a, box b)
    {
        return (abs(a.x() - b.x()) * 2 < (a.width() + b.width())) &&
               (abs(a.y() - b.y()) * 2 < (a.height() + b.height()));
    }
};

}

#endif //ZENGINE_COLLISION_HPP
