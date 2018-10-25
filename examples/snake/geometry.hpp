//
// Created by fotyev on 2018-10-18.
//

#ifndef ZENGINE_GEOMETRY_HPP
#define ZENGINE_GEOMETRY_HPP

#include "opengl/math.hpp"
#include "util/serialization.hpp"

using qvm::vec2;

struct segment_c
{
    // segment with direction
    // start: pos, end: pos + dir

    // as a line: f(t) = pos + dir * t

    vec2 pos;
    vec2 dir; // non-normalized

    vec2 operator()(float t) const { return pos + t * dir; }
    float length() const { return qvm::mag(dir); }

    SERIALIZABLE(pos,dir)
};

// 90 deg counterclockwise
vec2 rotate_left(vec2 p) { return vec2{-qvm::Y(p), qvm::X(p)}; }
// 90 deg clockwise
vec2 rotate_right(vec2 p) { return vec2{qvm::Y(p), -qvm::X(p)}; }

std::pair<float, float> intersect_lines(const segment_c& l1, const segment_c& l2)
{
    // l1 || l2
    // or
    // find t1, t2. l1(t1) == l2(t2)
    // l1.pos+l1.dir*t1=l2.pos+l2.dir*t2
    // l1.dir*t1-l2.dir*t2=l2.pos-l1.pos
    vec2 c1 = l1.dir, c2 = -l2.dir, cf = l2.pos - l1.pos;
    using namespace qvm;
    mat2 d;
    col<0>(d) = c1;
    col<1>(d) = c2;
    float D = determinant(d);
    col<0>(d) = cf;
    float D1 = determinant(d);
    col<0>(d) = c1;
    col<1>(d) = cf;
    float D2 = determinant(d);
    return {D1 / D, D2 / D};
}

bool intersect_segments(const segment_c& l1, const segment_c& l2, float eps = 0.000001)
{
    float t1, t2;
    std::tie(t1, t2) = intersect_lines(l1, l2);
    return t1 > 0.f + eps && t1 < 1.f - eps && t2 > 0.f + eps && t2 < 1.f - eps;
}

// returns t, such that l(t) is the closest to p
float closest_point_to_line(const segment_c& l, vec2 p)
{
    segment_c l2 {p, rotate_left(l.dir)}; // l2 is perpendicular to l, p is on l2
    return intersect_lines(l, l2).first;
}

float distance_to_line(const segment_c& line, vec2 p)
{
    return qvm::mag(p - line(closest_point_to_line(line, p)));
}

float distance_to_segment(const segment_c& line, vec2 p)
{
    return qvm::mag(p - line(
            qvm::clamp(closest_point_to_line(line, p), 0.f, 1.f)
            ));
}

#endif //ZENGINE_GEOMETRY_HPP
