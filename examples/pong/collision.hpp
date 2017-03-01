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

struct object
{

    virtual box bounding_box() const = 0;
    virtual void collide(object*) {}
};


class simple_collider
{
public:
    std::vector<object *> objs;

    static bool test(box a, box b)
    {
        return (abs(a.x() - b.x()) * 2 < (a.width() + b.width())) &&
               (abs(a.y() - b.y()) * 2 < (a.height() + b.height()));
    }

    void test_object(object * obj)
    {
        auto bbox = obj->bounding_box();
        std::for_each(objs.begin(), objs.end(), [obj, &bbox](auto other) {
            if(obj != other && test(bbox, other->bounding_box()))
            {
                obj->collide(other);
                //other->collide(obj);
            }
        });
    }
};

}

#endif //ZENGINE_COLLISION_HPP
