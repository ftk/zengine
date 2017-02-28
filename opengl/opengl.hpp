//
// Created by fotyev on 2016-10-29.
//

#ifndef ZENGINE_OPENGL_HPP
#define ZENGINE_OPENGL_HPP

/*
#include <SDL2/SDL_opengles2_khrplatform.h>
#include <SDL2/SDL_opengles2_gl2platform.h>
#include <SDL2/SDL_opengles2_gl2.h>
*/

#include <SDL2/SDL_video.h>

#include <stdexcept>

#include "gl2.h"
#include "util/movable.hpp"
#include "util/assert.hpp"

class gl_exc : public std::runtime_error
{
    GLenum error;
public:
    gl_exc();
    gl_exc(GLenum err);
};

class gl
{
    SDL_GLContext context = nullptr;

    NONCOPYABLE(gl)
public:
    gl(SDL_Window * window);
    ~gl();

#define GLFUNC(ret,name,params) static ret (*name) params;
#include "opengl.inl"
#undef GLFUNC

    static void feature(GLenum mask, bool toggle) { return (toggle ? gl::Enable(mask) : gl::Disable(mask)); }

};



#ifdef GL_USE_GLOBAL_FUNCTIONS

#define GLFUNC(ret,name,params) const auto& gl ## name = gl :: name;
#include "opengl.inl"
#undef GLFUNC

#endif


template <GLenum GLnum>
struct gl_to_type {};
template <typename T>
struct type_to_gl {};
/*< my %types = (
    GLfloat=>'GL_FLOAT',
    GLint=>'GL_INT',
    GLbyte=>'GL_BYTE',
    GLshort=>'GL_SHORT',

    GLuint=>'GL_UNSIGNED_INT',
    GLubyte=>'GL_UNSIGNED_BYTE',
    GLushort=>'GL_UNSIGNED_SHORT',
    );
    my $s = '';
    for my $type (sort keys %types) {
    $s .= "template<> struct gl_to_type<$types{$type}> { using type = $type; };\n";
    $s .= "template<> struct type_to_gl<$type> { static constexpr GLenum value = $types{$type}; };\n";
    }

    $s;
    %*/template<> struct gl_to_type<GL_BYTE> { using type = GLbyte; };
template<> struct type_to_gl<GLbyte> { static constexpr GLenum value = GL_BYTE; };
template<> struct gl_to_type<GL_FLOAT> { using type = GLfloat; };
template<> struct type_to_gl<GLfloat> { static constexpr GLenum value = GL_FLOAT; };
template<> struct gl_to_type<GL_INT> { using type = GLint; };
template<> struct type_to_gl<GLint> { static constexpr GLenum value = GL_INT; };
template<> struct gl_to_type<GL_SHORT> { using type = GLshort; };
template<> struct type_to_gl<GLshort> { static constexpr GLenum value = GL_SHORT; };
template<> struct gl_to_type<GL_UNSIGNED_BYTE> { using type = GLubyte; };
template<> struct type_to_gl<GLubyte> { static constexpr GLenum value = GL_UNSIGNED_BYTE; };
template<> struct gl_to_type<GL_UNSIGNED_INT> { using type = GLuint; };
template<> struct type_to_gl<GLuint> { static constexpr GLenum value = GL_UNSIGNED_INT; };
template<> struct gl_to_type<GL_UNSIGNED_SHORT> { using type = GLushort; };
template<> struct type_to_gl<GLushort> { static constexpr GLenum value = GL_UNSIGNED_SHORT; };
/*>*/


// 0 - dont check, 1 - assert, 2 - enable stderr log, 3 - log and assert
#ifndef GL_DEBUG
#ifdef NDEBUG
#define GL_DEBUG 0
#else
#define GL_DEBUG 3
#endif
#endif

#if (GL_DEBUG & 1)
#define GL_CHECK_ERROR() assert(gl::GetError() == GL_NO_ERROR)
#else
#define GL_CHECK_ERROR() ((void)0)
#endif



#endif //ZENGINE_OPENGL_HPP
