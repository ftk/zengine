//
// Created by fotyev on 2016-11-05.
//

#ifndef ZENGINE_TEXTURE_HPP
#define ZENGINE_TEXTURE_HPP


#include "util/movable.hpp"
#include "util/hash.hpp"
#include "math.hpp"

#include "gl2.h"


class texture
{
    unsigned idx = 0;
    unsigned width, height;

NONCOPYABLE_BUT_SWAPPABLE(texture, (idx) (width) (height))


public:
    // RGB 24bit or RGBA 32 bit sufrace.
    texture(const uint8_t * surface, unsigned surface_len, unsigned width, unsigned height, int format = -1);

    static texture from_file(const char * filename);
    texture(string_view filename);
    //texture() : size(-1,-1) {};

    // uninitialized texture
    explicit texture(unsigned width, unsigned height, int format = GL_RGB);

    // already created
    explicit texture(unsigned gl_idx, unsigned width, unsigned height) : idx(gl_idx), width(width), height(height) {}


    void bind(unsigned tex_unit = 0) const;

    ~texture();

    unsigned release() noexcept // disown texture
    {
        unsigned i = 0;
        std::swap(idx, i);
        return i;
    }

    unsigned get() const noexcept { return idx; }

    qvm::ivec2 get_size() const { return {(int)width, (int)height}; }
};

class framebuf
{ // todo: test
    unsigned buf = 0;
    unsigned rbo = 0;
public:
    texture tex;

    NONCOPYABLE(framebuf)
//NONCOPYABLE_BUT_SWAPPABLE(framebuf, (buf)(rbo)(tex))

public:
    framebuf(unsigned width, unsigned height);

    ~framebuf();

    void bind();

    static void unbind();

};

#include "util/resource_traits.hpp"

template <>
struct resource_traits<texture>
{
    static unsigned int get_size(const texture& rsc)
    {
        return static_cast<unsigned>(qvm::X(rsc.get_size()) * qvm::Y(rsc.get_size()) * 4);
    }
    static texture from_id(string_view id) { return texture{id}; }
};


#endif //ZENGINE_TEXTURE_HPP
