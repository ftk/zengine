//
// Created by fotyev on 2016-11-06.
//

#include "util/log.hpp"
#include "texture.hpp"

#include "opengl.hpp"
//#include "shader.hpp"



texture::texture(const uint8_t * surface, unsigned surface_len, unsigned width, unsigned height, int format)
: width(width), height(height)
{

    assume(width > 0);
    assume(height > 0);

    if(format == -1)
    {
        if(surface_len % (width * height) != 0)
            throw std::runtime_error{"texture: bad surface"};
        switch(surface_len / (width * height))
        {
            case 1:
                format = GL_ALPHA;
                break;
            case 2:
                format = GL_LUMINANCE_ALPHA;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                throw std::runtime_error{"texture: bad surface"};
        }
    }


    gl::GenTextures(1, &idx);
    GL_CHECK_ERROR();

    gl::BindTexture(GL_TEXTURE_2D, idx);
    GL_CHECK_ERROR();


    // GL_NEAREST GL_LINEAR
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // GL_MIRRORED_REPEAT, GL_REPEAT GL_CLAMP_TO_EDGE
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR();


    /*   texels are read from memory, starting at location data.
     * By default, these texels are taken from adjacent memory locations, except that after all width texels are read,
     * the read pointer is advanced to the next four-byte boundary.
     * The four-byte row alignment is specified by glPixelStorei with argument GL_UNPACK_ALIGNMENT, and it can be set
     * to one, two, four, or eight bytes. */
    // http://docs.gl/es2/glTexImage2D

    if((format == GL_RGB && (width * 3) % 4 != 0)
       || (format == GL_ALPHA && (width) % 4 != 0)
       || (format == GL_LUMINANCE_ALPHA && (width * 2) % 4 != 0)
    )
    {
        gl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    gl::TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, surface);
    GL_CHECK_ERROR();

    LOGGER(debug, "loaded texture", idx, width, height, format);

}

void texture::bind(unsigned tex_unit) const
{
    assume(idx);
    gl::ActiveTexture(GL_TEXTURE0 + tex_unit);
    gl::BindTexture(GL_TEXTURE_2D, idx);
    GL_CHECK_ERROR();
}

texture::~texture()
{
    if(gl::initialized && idx)
    {
        gl::DeleteTextures(1, &idx);
        GL_CHECK_ERROR();
        LOGGER(debug, "unloaded texture", idx);
    }
}



texture::texture(unsigned width, unsigned height, int format) : width(width), height(height)
{
    gl::GenTextures(1, &idx);
    GL_CHECK_ERROR();

    gl::BindTexture(GL_TEXTURE_2D, idx);
    GL_CHECK_ERROR();


    // GL_NEAREST GL_LINEAR
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // GL_MIRRORED_REPEAT, GL_REPEAT GL_CLAMP_TO_EDGE
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR();

    gl::TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
    GL_CHECK_ERROR();

    //gl::BindTexture(GL_TEXTURE_2D, 0);
    //GL_CHECK_ERROR();

}

#include "stb/stb_image.h"

#include <boost/scope_exit.hpp>

texture texture::from_file(const char * filename)
{
    int width = 0, height, comp;
    auto img = stbi_load(filename, &width, &height, &comp, 0);
    if(!img)
        throw std::runtime_error{"cant load " + std::string{filename}};
    assume(width > 0);
    BOOST_SCOPE_EXIT_ALL(&img) { stbi_image_free(img); };
    return texture{img, (unsigned)width * height * comp, (unsigned)width, (unsigned)height};
}

texture::texture(string_view filename) : texture(from_file(std::string(filename).c_str()))
{
}

framebuf::framebuf(unsigned width, unsigned height, bool depth) : tex(width, height, GL_RGBA)
{
#if GLES_VERSION >= 3
    // https://learnopengl.com/code_viewer.php?code=in-practice/breakout/post_processor
    {
        gl::GenRenderbuffers(1, &rbo);
        gl::GenFramebuffers(1, &msfbo);
        gl::BindFramebuffer(GL_FRAMEBUFFER, msfbo);
        gl::BindRenderbuffer(GL_RENDERBUFFER, rbo);
        GL_CHECK_ERROR();


        gl::RenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, width, height);
        GL_CHECK_ERROR();
        gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        GL_CHECK_ERROR();
        assert(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }
#endif

    gl::GenFramebuffers(1, &fbo);
    gl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    GL_CHECK_ERROR();
    tex.bind();
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.get(), 0);
    GL_CHECK_ERROR();

    if(depth)
    {
#if GLES_VERSION >= 3
        gl::BindFramebuffer(GL_FRAMEBUFFER, msfbo);
#endif
        gl::GenRenderbuffers(1, &depth_rbo);
        gl::BindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
        // TODO: add stencil buffer
        gl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
        GL_CHECK_ERROR();
    }

    assert(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    unbind();
}

framebuf::~framebuf()
{
    if(gl::initialized)
    {
        gl::DeleteFramebuffers(1, &fbo);
#if GLES_VERSION >= 3
        gl::DeleteFramebuffers(1, &msfbo);
        gl::DeleteRenderbuffers(1, &rbo);
#endif
        if(depth_rbo)
            gl::DeleteRenderbuffers(1, &depth_rbo);

        GL_CHECK_ERROR();
    }
}

void framebuf::bind()
{
    gl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    GL_CHECK_ERROR();
}

void framebuf::unbind()
{
    gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERROR();
}

void framebuf::render_begin(bool clear)
{
#if GLES_VERSION >= 3
    gl::BindFramebuffer(GL_FRAMEBUFFER, msfbo);
#else
    gl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
#endif
    gl::Viewport(0, 0, qvm::X(get_size()), qvm::Y(get_size()));
    if(clear)
        gl::Clear(GL_COLOR_BUFFER_BIT | (depth_rbo ? GL_DEPTH_BUFFER_BIT : 0));
    GL_CHECK_ERROR();
}

void framebuf::render_end()
{
#if GLES_VERSION >= 3
    using namespace qvm;
    gl::BindFramebuffer(GL_READ_FRAMEBUFFER, msfbo);
    gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    gl::BlitFramebuffer(0, 0, X(get_size()), Y(get_size()), 0, 0, X(get_size()), Y(get_size()), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GL_CHECK_ERROR();
#endif
    unbind();

}

#include "util/string.hpp"
#include "util/hash.hpp"

texture resource_traits<texture>::from_id(string_view id)
{
    auto v = string_split_v(id, '#');
    assume(!v.empty());

    auto tex = texture{v[0]};
    // apply parameters
    for(unsigned i = 1; i < v.size(); i++)
    {
        auto kv = string_split<2>(v[i], '=');
        GLenum key = 0, value = 0;
        switch(fnv1a::hash(kv[0]))
        {
            /*< join "\n", map { qq[case "${_}"_fnv: key = GL_TEXTURE_${_}; break;] }
                qw(MIN_FILTER MAG_FILTER WRAP_S WRAP_T); %*/case "MIN_FILTER"_fnv: key = GL_TEXTURE_MIN_FILTER; break;
case "MAG_FILTER"_fnv: key = GL_TEXTURE_MAG_FILTER; break;
case "WRAP_S"_fnv: key = GL_TEXTURE_WRAP_S; break;
case "WRAP_T"_fnv: key = GL_TEXTURE_WRAP_T; break;/*>*/
        }
        if(!key)
            throw std::runtime_error{"texture: unknown param " + std::string{kv[0]}};
        switch(fnv1a::hash(kv[1]))
        {
            /*< join "\n", map { qq[case "${_}"_fnv: value = GL_${_}; break;] }
                qw(NEAREST LINEAR NEAREST_MIPMAP_NEAREST LINEAR_MIPMAP_NEAREST NEAREST_MIPMAP_LINEAR LINEAR_MIPMAP_LINEAR
                CLAMP_TO_EDGE MIRRORED_REPEAT REPEAT); %*/case "NEAREST"_fnv: value = GL_NEAREST; break;
case "LINEAR"_fnv: value = GL_LINEAR; break;
case "NEAREST_MIPMAP_NEAREST"_fnv: value = GL_NEAREST_MIPMAP_NEAREST; break;
case "LINEAR_MIPMAP_NEAREST"_fnv: value = GL_LINEAR_MIPMAP_NEAREST; break;
case "NEAREST_MIPMAP_LINEAR"_fnv: value = GL_NEAREST_MIPMAP_LINEAR; break;
case "LINEAR_MIPMAP_LINEAR"_fnv: value = GL_LINEAR_MIPMAP_LINEAR; break;
case "CLAMP_TO_EDGE"_fnv: value = GL_CLAMP_TO_EDGE; break;
case "MIRRORED_REPEAT"_fnv: value = GL_MIRRORED_REPEAT; break;
case "REPEAT"_fnv: value = GL_REPEAT; break;/*>*/
        }
        if(!value)
            throw std::runtime_error{"texture: unknown param value " + std::string{kv[1]}};
        gl::TexParameteri(GL_TEXTURE_2D, key, value);
        GL_CHECK_ERROR();
    }

    return tex;
}
