//
// Created by fotyev on 2016-11-06.
//

#include "texture.hpp"

#include "opengl.hpp"
//#include "shader.hpp"

#include "util/geometry.hpp"
#include <SDL2pp/Surface.hh>
#include <SDL2/SDL_pixels.h>

#include "util/sdl_workaround.hpp"

texture::texture(SDL2pp::Surface&& surface)
{
    if(surface.GetFormat() != SDL_PIXELFORMAT_RGB24 && surface.GetFormat() != SDL_PIXELFORMAT_RGBA32)
        throw std::runtime_error{"bad surface format, please convert: " + std::to_string(surface.GetFormat())};


    size = surface.GetSize();

    assume(size.x > 0);
    assume(size.y > 0);


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

    auto lock = surface.Lock();

    /*   texels are read from memory, starting at location data.
     * By default, these texels are taken from adjacent memory locations, except that after all width texels are read,
     * the read pointer is advanced to the next four-byte boundary.
     * The four-byte row alignment is specified by glPixelStorei with argument GL_UNPACK_ALIGNMENT, and it can be set
     * to one, two, four, or eight bytes. */
    // http://docs.gl/es2/glTexImage2D
    const GLenum format = (surface.GetFormat() == SDL_PIXELFORMAT_RGB24) ? GL_RGB : GL_RGBA;

    if(format == GL_RGB) // rgba has 4 bytes per pixel -- already aligned
    {
        unsigned desired_pitch = size.x * 3;
        // align to 4-byte boundary
        desired_pitch = (((desired_pitch - 1) / 4) + 1) * 4;
        if((unsigned)lock.GetPitch() != desired_pitch)
        {
            throw std::runtime_error{"bad pitch: " + std::to_string(lock.GetPitch()) + ", need: " + std::to_string(desired_pitch)};
        }
    }

    gl::TexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, GL_UNSIGNED_BYTE, lock.GetPixels());
    GL_CHECK_ERROR();

    gl::BindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERROR();

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
    if(idx)
    {
        gl::DeleteTextures(1, &idx);
        GL_CHECK_ERROR();
    }
}

texture texture::from_surface(SDL2pp::Surface&& surf)
{
    auto fmt = surf.GetFormat();
    if(fmt != SDL_PIXELFORMAT_RGBA32 && fmt != SDL_PIXELFORMAT_RGB24)
        surf = SDL_ISPIXELFORMAT_ALPHA(fmt) ? surf.Convert(SDL_PIXELFORMAT_RGBA32) : surf.Convert(SDL_PIXELFORMAT_RGB24);
    return texture{std::move(surf)};
}

texture texture::from_file(string_view filename)
{
    return from_surface(SDL2pp::Surface{filename.to_string()});
}

texture::texture(string_view filename) : texture(SDL2pp::Surface{filename.to_string()})
{
}

void texture::set_params(unsigned mag, unsigned min, unsigned wrap_s, unsigned wrap_t)
{
    assume(idx);
    gl::BindTexture(GL_TEXTURE_2D, idx);
    GL_CHECK_ERROR();


    // GL_NEAREST GL_LINEAR
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    GL_CHECK_ERROR();
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
    GL_CHECK_ERROR();
    // GL_MIRRORED_REPEAT, GL_REPEAT GL_CLAMP_TO_EDGE
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    GL_CHECK_ERROR();
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    GL_CHECK_ERROR();

    gl::BindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERROR();

}



texture::texture(Point size, bool alpha) : size(size)
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

    const GLenum format = alpha ? GL_RGBA : GL_RGB;
    gl::TexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, GL_UNSIGNED_BYTE, NULL);
    GL_CHECK_ERROR();

    gl::BindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERROR();

}

framebuf::framebuf(Point size) : tex(size, false)
{
    gl::GenFramebuffers(1, &buf);
    GL_CHECK_ERROR();
    bind();

    gl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.get(), 0);
    GL_CHECK_ERROR();

    gl::GenRenderbuffers(1, &rbo);
    gl::BindRenderbuffer(GL_RENDERBUFFER, rbo);
    // TODO: add stencil buffer
    gl::RenderbufferStorage(GL_RENDERBUFFER, /*GL_DEPTH24_STENCIL8*/GL_DEPTH_COMPONENT16, size.x, size.y);
    gl::BindRenderbuffer(GL_RENDERBUFFER, 0);
    gl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    GL_CHECK_ERROR();

    assert(gl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    unbind();
}

framebuf::~framebuf()
{
    gl::DeleteFramebuffers(1, &buf);
    //gl::DeleteTextures(1, &tex);
    gl::DeleteRenderbuffers(1, &rbo);
    GL_CHECK_ERROR();
}

void framebuf::bind()
{
    gl::BindFramebuffer(GL_FRAMEBUFFER, buf);
    GL_CHECK_ERROR();
}

void framebuf::unbind()
{
    gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERROR();
}

SDL2pp::Surface create_optimized_surface(Point size)
{
    // SDL_PIXELFORMAT_RGBA32
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL2pp::Surface(0, size.x, size.y, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
    return SDL2pp::Surface(0, size.x, size.y, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
}
