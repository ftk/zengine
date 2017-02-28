//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_GEOMETRY_HPP
#define ZENGINE_GEOMETRY_HPP

#include <SDL2pp/Point.hh>
#include <SDL2pp/Rect.hh>


using SDL2pp::Point;
using SDL2pp::Rect;


//#include <boost/qvm/all.hpp>

#include <boost/qvm/vec_traits.hpp>

namespace boost
{
namespace qvm
{

template <>
struct vec_traits<Point>
{
    static int const dim = 2;
    typedef int scalar_type;

    template <int I>
    static inline scalar_type & write_element(Point& v);

    template <int I>
    static inline scalar_type read_element(const Point& v);

};
template <>
inline vec_traits<Point>::scalar_type & vec_traits<Point>::write_element<0>(Point& v) { return v.x; }
template <>
inline vec_traits<Point>::scalar_type & vec_traits<Point>::write_element<1>(Point& v) { return v.y; }
template <>
inline vec_traits<Point>::scalar_type vec_traits<Point>::read_element<0>(const Point& v) { return v.x; }
template <>
inline vec_traits<Point>::scalar_type vec_traits<Point>::read_element<1>(const Point& v) { return v.y; }

}
}


#endif //ZENGINE_GEOMETRY_HPP
