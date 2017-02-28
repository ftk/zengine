//
// Created by fotyev on 2016-10-29.
//

#include "opengl.hpp"
//#include <SDL2/SDL_opengl.h>

#include <string>

#include "util/log.hpp"
/*# wrapper example
   my $s='';
   open my $fh, '<', 'opengl/opengl.inl';
   while(<$fh>)
   {
    if(m/GLFUNC\((.+?), *(.+?), *\((.*?)\)\)/)
    {
      my @params = split /, /, $3;
      my $arglist = join ', ', map { (split / /)[-1]; } @params;
      $s .= qq[static $1 $2 ($3) {
        logger(log_level::debug, "gl$2", '(', $arglist, ')');
        return real$2($arglist);
        }];

    }
   }
   #$s;

 %*/ /*>*/

#if GL_DEBUG >= 2

#ifndef GL_DEBUG_LOG
#define GL_DEBUG_LOG 3 // 1 high, 2 medium, 3 low, 4 notification
#endif


#include <iostream>


//#include "gl2ext.h"
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR 0x826B
#define GL_DEBUG_SEVERITY_HIGH_KHR        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_KHR      0x9147
#define GL_DEBUG_SEVERITY_LOW_KHR         0x9148

typedef void (*DEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);

static void debug_log(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
#if GL_DEBUG_LOG < 4
            return;
#endif
        case GL_DEBUG_SEVERITY_LOW_KHR:
#if GL_DEBUG_LOG < 3
            return;
#endif
        case GL_DEBUG_SEVERITY_MEDIUM_KHR:
#if GL_DEBUG_LOG < 2
            return;
#endif
        case GL_DEBUG_SEVERITY_HIGH_KHR:
#if GL_DEBUG_LOG < 1
            return;
#endif
        default:
            break;

    }
    //std::cerr << source << ' ' << type << ' ' << id << ' ' << severity << ' ';
    std::cerr.write(message, length);
    std::cerr << std::endl;

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



