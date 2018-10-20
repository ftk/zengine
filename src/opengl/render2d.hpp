//
// Created by fotyev on 2016-11-16.
//

#ifndef ZENGINE_RENDER2D_HPP
#define ZENGINE_RENDER2D_HPP

#include "shader.hpp"
#include "texture.hpp"
#include <array>
#include <vector>

#include <boost/noncopyable.hpp>

/**
 * Renderer for 2D textures and sprites
 * Draws from lower left corner
 */
class renderer_2d
{
public:
    program shader;

private:
    /*< define_attributes('attributes',{
a_position=>['qvm::vec2','float'],
a_texCoord=>['qvm::vec2','float'],
});%*/
#pragma pack(push,1)
struct attributes
{
qvm::vec2 	a_position;
qvm::vec2 	a_texCoord;
static void setup(program& p) {
    GET_ATTRIBUTE(p,a_position).setup(sizeof(qvm::vec2)/sizeof(float), type_to_gl<float>::value, false, sizeof(attributes), offsetof(attributes, a_position));
    GET_ATTRIBUTE(p,a_texCoord).setup(sizeof(qvm::vec2)/sizeof(float), type_to_gl<float>::value, false, sizeof(attributes), offsetof(attributes, a_texCoord));
}
static void disable(program& p) {
    GET_ATTRIBUTE(p,a_position).disable();
    GET_ATTRIBUTE(p,a_texCoord).disable();
}
};
#pragma pack(pop)
/*>*/
private:
    std::array<attributes, 4> vertices;
    static_assert(sizeof(vertices) == sizeof(attributes) * 4);

    array_buf vertices_buf;

public:

    renderer_2d(const char * shader_filename) : shader{program::from_file(shader_filename)},
                    vertices_buf{&vertices[0], sizeof(vertices), GL_DYNAMIC_DRAW}
    {
        // bind texture to 0 (always)
        shader.bind();
        GET_UNIFORM(shader, u_texture) = qvm::ivec1{0};
    }


private:
    void copy_from_custom_buf(texture& tex, GLenum mode, unsigned first, unsigned count)
    {
        shader.bind();
        tex.bind(0);
        shader.draw<attributes>(mode, first, count);
    }
public:
    // lower left corners and size
    void copy(texture &tex,
              qvm::vec2 ll = {0.f, 0.f},
              qvm::vec2 size = {2.f, 2.f},
              qvm::vec2 src_ll = {0.f, 0.f},
              qvm::vec2 src_size = {1.f, 1.f}
    )
    {
        using namespace qvm;
        // ul
        const float up = Y(src_ll);
        const float left = X(src_ll);
        vertices[0].a_position = XY(ll) + _0Y(size);
        vertices[0].a_texCoord = vec2{left,up};
        // ll
        const float low = Y(src_ll + src_size);
        vertices[1].a_position = ll;
        vertices[1].a_texCoord = vec2{left,low};
        // ur
        const float right = X(src_ll + src_size);
        vertices[2].a_position = ll + size;
        vertices[2].a_texCoord = vec2{right,up};
        // lr
        vertices[3].a_position = XY(ll) + X0(size);
        vertices[3].a_texCoord = vec2{right,low};

        vertices_buf.update(vertices.data(), sizeof(vertices)); // binded

        copy_from_custom_buf(tex, GL_TRIANGLE_STRIP, 0, vertices.size());
    }

    // calculate x preserving aspect ratio of texture
    void copy_y(texture& tex, qvm::vec2 ll, float sizey)
    {
        using namespace qvm;
        auto size = tex.get_size();
        return copy(tex, ll, {(float(X(size)) / Y(size)) * sizey, sizey});
    }
};

// draws colored rectangles, requires flush (1 draw call)
class rectanges_renderer : boost::noncopyable
{
public:
    program shader;

private:
/*< define_attributes('attributes',{
a_position=>['qvm::vec2','float'],
a_color=>['uint32_t','uint8_t','true'],
});%*/
#pragma pack(push,1)
struct attributes
{
uint32_t 	a_color;
qvm::vec2 	a_position;
static void setup(program& p) {
    GET_ATTRIBUTE(p,a_color).setup(sizeof(uint32_t)/sizeof(uint8_t), type_to_gl<uint8_t>::value, true, sizeof(attributes), offsetof(attributes, a_color));
    GET_ATTRIBUTE(p,a_position).setup(sizeof(qvm::vec2)/sizeof(float), type_to_gl<float>::value, false, sizeof(attributes), offsetof(attributes, a_position));
}
static void disable(program& p) {
    GET_ATTRIBUTE(p,a_color).disable();
    GET_ATTRIBUTE(p,a_position).disable();
}
};
#pragma pack(pop)
/*>*/
public:
    std::vector<attributes> vertices;
private:
    array_buf vertices_buf;
public:
    rectanges_renderer(const char * shader_filename) : shader{program::from_file(shader_filename)} {}

    void add_rect(qvm::vec2 ll, qvm::vec2 size, uint32_t color)
    {
        vertices.push_back({color, XY(ll) + _0Y(size)}); // 1
        vertices.push_back({color, ll}); // 2
        vertices.push_back({color, ll + size}); // 3
        vertices.push_back({color, ll + size}); // 3
        vertices.push_back({color, ll}); // 2
        vertices.push_back({color, XY(ll) + X0(size)}); // 4
    }

    void draw()
    {
        if(vertices.empty())
            return;
        shader.bind();
        vertices_buf.set(vertices.data(), vertices.size() * sizeof(attributes), GL_DYNAMIC_DRAW);
        shader.draw<attributes>(GL_TRIANGLES, 0, vertices.size());
    }
    void clear()
    {
        vertices.clear();
    }

    void flush()
    {
        draw();
        clear();
    }
};

/* Renderer for toy fragment shaders (renders whole screen)
 * usage:
 * toy_renderer r{"file.glsl"};
 * r.shader.bind();
 * GET_UNIFORM(r.shader, u_time) = qvm::vec1{(float)glfwGetTime()};
 * r.draw();
 *
 */
class toy_renderer : boost::noncopyable
{
    /*< define_attributes('attributes',{
a_position=>['qvm::vec2','float'],
});%*/
#pragma pack(push,1)
struct attributes
{
qvm::vec2 	a_position;
static void setup(program& p) {
    GET_ATTRIBUTE(p,a_position).setup(sizeof(qvm::vec2)/sizeof(float), type_to_gl<float>::value, false, sizeof(attributes), offsetof(attributes, a_position));
}
static void disable(program& p) {
    GET_ATTRIBUTE(p,a_position).disable();
}
};
#pragma pack(pop)
/*>*/


    array_buf buf;
public:
    program shader;
public:

    toy_renderer(const char * filename) : shader{program::from_file(filename)}
    {
        using qvm::vec2;
        const attributes attr[4] = {{vec2{-1,1}}, {vec2{-1,-1}}, {vec2{1,1}}, {vec2{1,-1}}};
        buf.set(attr, sizeof(attr), GL_STATIC_DRAW);
    }

    void draw()
    {
        buf.bind();
        shader.draw<attributes>(GL_TRIANGLE_STRIP, 0, 4);
    }
};


#endif //ZENGINE_RENDER2D_HPP
