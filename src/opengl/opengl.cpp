//
// Created by fotyev on 2016-10-29.
//

#include "opengl.hpp"
#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>

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

bool gl::initialized = false;

gl::gl()
{
        // initialize
        assume(!initialized);

        unsigned extloaded = 0, extfailed = 0;

#define GLFUNC(ret,name,params) \
        this->name = reinterpret_cast<ret (*) params>(glfwGetProcAddress("gl" #name)); \
        if(this->name == nullptr) throw std::runtime_error{"Function " + std::string{"gl" #name} + " is null"};
#include GLFUNC_FILE
#undef GLFUNC

#ifdef GLES_EXTENSIONS
#define GLFUNC(ret,name,params) \
        this->name = reinterpret_cast<ret (*) params>(glfwGetProcAddress("gl" #name)); \
        if(this->name) extloaded++; else extfailed++;
#include "openglext.inl"
#undef GLFUNC
    LOGGER(info, "OpenGL ES", GLES_VERSION, "extensions loaded:", extloaded, "failed:", extfailed);
#endif


#if GL_DEBUG >= 2

    auto glDebugMessageCallback = reinterpret_cast<void (*) (DEBUGPROC callback, void* userParam)>(glfwGetProcAddress("glDebugMessageCallback"));
    if(glDebugMessageCallback)
    {
        glDebugMessageCallback(&debug_log, nullptr);
    }
    else
    {
        debug_log(0,0,0,0,sizeof("log is disabled")-1, "log is disabled", nullptr);
    }

#endif
    initialized = true;
}

gl::~gl()
{
    assume(initialized);
    initialized = false;
#if 0
#define GLFUNC(ret,name,params) this->name = nullptr;
#include GLFUNC_FILE
#ifdef GLES_EXTENSIONS
#include "openglext.inl"
#endif
#undef GLFUNC
#endif
}

// initialize static variables
#define GLFUNC(ret,name,params) ret (*gl::name) params = nullptr;
#include GLFUNC_FILE
#ifdef GLES_EXTENSIONS
#include "openglext.inl"
#endif
#undef GLFUNC

/*
  $filehandlers{'glfuncs'} = sub {
     my ($fn, $content) = @_;
     while ($$content =~ m{gl::([A-Z]\w*)}ga) {
         collect('glfuncs', $1);
     }
  };


  make_include('generated/opengl.txt', join("\n", map { '\b' . $_ . '\b' } dispatch_s('glfuncs')));
  system('grep -h -f generated/opengl.txt src/opengl/opengl*.inl > generated/opengl.inl') if ($pass == 2);
 */