//
// Created by fotyev on 2018-07-06.
//

#ifndef ZENGINE_GLFW_HPP
#define ZENGINE_GLFW_HPP

#include "util/log.hpp"

#include <GLFW/glfw3.h>


//=- register_component(class=>'glfw_c', name=>'glfw_init', priority=>5);
// depends on: modules


class glfw_c
{
    static void glfw_logger(int code, const char * errormsg)
    {
        LOGGER(error, "glfw error", code, errormsg);
    }

public:
    glfw_c()
    {
        glfwSetErrorCallback(&glfw_logger);
        if(!glfwInit())
            throw std::runtime_error{"glfw init failed"};

    }

    ~glfw_c()
    {
        glfwTerminate();
    }

};

#endif //ZENGINE_GLFW_HPP
