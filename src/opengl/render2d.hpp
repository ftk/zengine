//
// Created by fotyev on 2016-11-16.
//

#ifndef ZENGINE_RENDER2D_HPP
#define ZENGINE_RENDER2D_HPP

#include "shader.hpp"
#include "texture.hpp"
#include <array>
#include <vector>

#include "util/geometry.hpp"
#include "util/optional.hpp"


#include <boost/noncopyable.hpp>

// pimpl, maybe?
class renderer_2d : boost::noncopyable
{
    program shd;

public:
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

private:
    typedef std::function<qvm::vec2 (qvm::vec2)> transormation2d;
    std::vector<transormation2d> transforms;
public:
    struct transform_t
    {
        renderer_2d& ren;
        transform_t(renderer_2d& ren, auto f) : ren(ren)
        {
            ren.transforms.emplace_back(std::move(f));
        }
        ~transform_t()
        {
            ren.transforms.pop_back();
        }
        static auto rect(qvm::vec2 ll, qvm::vec2 size)
        {
            return [ll,size](qvm::vec2 pos) -> qvm::vec2 {
                return pos * qvm::diag_mat(size) + ll;
            };
        }
    };
    friend struct transform_t;
public:
    transform_t transform(auto f) { return {*this, std::move(f)}; }
public:
    renderer_2d() : shd{program::from_file("resources/shd/texture.glsl")}, // TODO: move from file to source?
                    vertices_buf{&vertices[0], sizeof(vertices), GL_DYNAMIC_DRAW}
    {
        set4dpos();
        // bind texture to 0 (always)
        GET_UNIFORM(shd, u_texture) = qvm::ivec1{0};

    }



    void copy_from_custom_buf(texture& tex, GLenum mode, unsigned first, unsigned count)
    {
        shd.bind();
        tex.bind(0);
        shd.draw<attributes>(mode, first, count);
    }

    // lower left and upper right corners
    void copy(texture &tex, qvm::vec2 ll = {0.f, 0.f}, qvm::vec2 ur = {1.f, 1.f}, optional<Rect> src = nullopt)
    {
        using namespace qvm;
        auto size = tex.get_size();

        for(const auto& f : transforms)
        {
            ll = f(ll);
            ur = f(ur);
        }

        // ul
        const float up = src ? ((float)src->y / size.y) : 0.f;
        const float left = src ? ((float)src->x / size.x) : 0.f;
        vertices[0].a_position = _0Y(ur) + X0(ll);
        vertices[0].a_texCoord = vec2{left,up};
        // ll
        const float low = src ? ((float)(src->y + src->h) / size.y) : 1.f;
        vertices[1].a_position = ll;
        vertices[1].a_texCoord = vec2{left,low};
        // ur
        const float right = src ? ((float)(src->x + src->w) / size.x) : 1.f;
        vertices[2].a_position = ur;
        vertices[2].a_texCoord = vec2{right,up};
        // lr
        vertices[3].a_position = X0(ur) + _0Y(ll);
        vertices[3].a_texCoord = vec2{right,low};

        vertices_buf.update(vertices.data(), sizeof(vertices)); // binded

        copy_from_custom_buf(tex, GL_TRIANGLE_STRIP, 0, vertices.size());
    }

    void copy2(texture& tex, qvm::vec2 ll = {0.f, 0.f}, qvm::vec2 size = {1.f, 1.f}, optional<Rect> src = nullopt)
    {
        return copy(tex, ll, ll + size, src);
    }

    // calculate x preserving aspect ratio of texture
    void copy_y(texture& tex, qvm::vec2 ll, float sizey)
    {
        auto size = tex.get_size();
        return copy2(tex, ll, {((float)size.x / size.y) * sizey, sizey});
    }

    void set4dpos(qvm::vec4 pos = qvm::vec4{0, 0, 0, 1}, float scale = 1)
    {
        shd.bind();
        GET_UNIFORM(shd, u_offset) = pos;
        GET_UNIFORM(shd, u_scale) = qvm::vec1{scale};
    }

};



#endif //ZENGINE_RENDER2D_HPP
