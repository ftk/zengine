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
    GLuint fbo = 0;
#if GLES_VERSION >= 3
    GLuint msfbo = 0;
    GLuint rbo = 0;
#endif
    GLuint depth_rbo = 0;
public:
    texture tex;

    NONCOPYABLE(framebuf)

public:
    framebuf(unsigned width, unsigned height, bool depth = false);
    framebuf(qvm::ivec2 size, bool depth = false) : framebuf(qvm::X(size), qvm::Y(size), depth) {}

    ~framebuf();

    void bind();

    void render_begin(bool clear = true);
    void render_end();

    static void unbind();

    qvm::ivec2 get_size() const { return tex.get_size(); }

};

#include "util/resource_traits.hpp"

template <>
struct resource_traits<texture>
{
    static unsigned int get_size(const texture& rsc)
    {
        return static_cast<unsigned>(qvm::X(rsc.get_size()) * qvm::Y(rsc.get_size()) * 4);
    }
    /**
     * @param id texture with optional texture params: "test.png#MAG_FILTER=NEAREST#MIN_FILTER=LINEAR"
     */
    static texture from_id(string_view id);
};



#endif //ZENGINE_TEXTURE_HPP
