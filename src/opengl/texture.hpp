//
// Created by fotyev on 2016-11-05.
//

#ifndef ZENGINE_TEXTURE_HPP
#define ZENGINE_TEXTURE_HPP


#include "util/movable.hpp"
#include "util/geometry.hpp"
#include "util/hash.hpp"

#include "resource.hpp"


namespace SDL2pp { class Surface; }

class texture : public gl_resource
{
    unsigned idx = 0;
    Point size;

NONCOPYABLE_BUT_SWAPPABLE(texture, (idx) (size))


public:
    texture(SDL2pp::Surface&& surface);
    texture(string_view filename);
    //texture() : size(-1,-1) {};

    // uninitialized texture
    explicit texture(Point size, bool alpha = false);

    // already created
    explicit texture(unsigned gl_idx, Point size) : idx(gl_idx), size(size) {}


    static texture from_surface(SDL2pp::Surface&& surface);
    static texture from_file(string_view filename);


    //texture(texture&& rhs) noexcept { std::swap(idx, rhs.idx); std::swap(size, rhs.size); };


    void bind(unsigned tex_unit = 0) const;

    Point get_size() const { return size; }

    void set_params(unsigned mag, unsigned min, unsigned wrap_s, unsigned wrap_t);

    ~texture();

    unsigned release() noexcept // disown texture
    {
        unsigned i = 0;
        std::swap(idx, i);
        return i;
    }

    unsigned get() const noexcept { return idx; }

};

SDL2pp::Surface create_optimized_surface(Point size);
//SDL2pp::Surface create_optimized_surface_noalpha(Point size);


class framebuf : public gl_resource
{ // todo: test
    unsigned buf = 0;
    unsigned rbo = 0;
public:
    texture tex;

    NONCOPYABLE(framebuf)
//NONCOPYABLE_BUT_SWAPPABLE(framebuf, (buf)(rbo)(tex))

public:
    framebuf(Point size);

    ~framebuf();

    void bind();

    static void unbind();

};

#endif //ZENGINE_TEXTURE_HPP
