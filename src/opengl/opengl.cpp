//
// Created by fotyev on 2016-10-29.
//

#include "opengl.hpp"
//#include <SDL_opengl.h>

#include <string>

#include "util/assert.hpp"



#if GL_DEBUG >= 2

//#include <iostream>
#include "util/log.hpp"

//#include "gl2ext.h"
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR 0x826B
#define GL_DEBUG_SEVERITY_HIGH_KHR        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_KHR      0x9147
#define GL_DEBUG_SEVERITY_LOW_KHR         0x9148

typedef void (*DEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);

static void debug_log(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    auto lvl = log_level::error;
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
            lvl = log_level::debug;
            break;
        case GL_DEBUG_SEVERITY_LOW_KHR:
            lvl = log_level::info;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM_KHR:
            lvl = log_level::warn;
            break;
        case GL_DEBUG_SEVERITY_HIGH_KHR:
            lvl = log_level::error;
            break;
        default:
            break;

    }
    //std::cerr << source << ' ' << type << ' ' << id << ' ' << severity << ' ';
    //std::cerr.write(message, length);
    //std::cerr << std::endl;
    assume(length >= 0);
    logger(lvl, "OpenGL:", string_view{message,static_cast<std::size_t>(length)});
}

#endif


gl::gl(SDL_Window * window)
{
    // opengl es 2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#if GL_DEBUG >= 2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    context = SDL_GL_CreateContext(window);
    if(!context)
        throw std::runtime_error(SDL_GetError());

    if(SDL_GL_MakeCurrent(window, context) < 0)
        throw std::runtime_error(SDL_GetError());

        // initialize


#define GLFUNC(ret,name,params) \
        this->name = reinterpret_cast<ret (*) params>(SDL_GL_GetProcAddress("gl" #name)); \
        if(this->name == nullptr) throw std::runtime_error{"Function " "gl" #name " is null"};
#include "opengl.inl"
#undef GLFUNC



#if GL_DEBUG >= 2

    auto glDebugMessageCallback = reinterpret_cast<void (*) (DEBUGPROC callback, void* userParam)>(SDL_GL_GetProcAddress("glDebugMessageCallback"));
    if(glDebugMessageCallback)
    {
        glDebugMessageCallback(&debug_log, nullptr);
    }
    else
    {
        debug_log(0,0,0,0,sizeof("log is disabled")-1, "log is disabled", nullptr);
    }

#endif
}

gl::~gl()
{
    if(context)
        SDL_GL_DeleteContext(context);
}

// initialize static variables
#define GLFUNC(ret,name,params) ret (*gl::name) params = nullptr;
#include "opengl.inl"
#undef GLFUNC

gl_exc::gl_exc() : std::runtime_error("OpenGL error!")
{
    error = gl::GetError();
}
gl_exc::gl_exc(GLenum err) : std::runtime_error("OpenGL error! " + std::to_string(err)), error(err)
{
}



