//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_WINDOW_HPP
#define ZENGINE_WINDOW_HPP

#include <GLFW/glfw3.h>

#include "opengl/opengl.hpp"
#include "opengl/render2d.hpp"
#include "opengl/math.hpp"

#include "util/signals.hpp"

//=- register_component(class=>'window_c', name=>'window', priority=>10);

// opengl window
class window_c
{
    struct window_handle
    {
        GLFWwindow * h;

        window_handle(int w, int h, const char * title);
        ~window_handle();
    } window;
    gl ctx;
public:
    renderer_2d render { "resources/shd/texture.glsl" };
    renderer_2d text_render { "resources/shd/smoothtext.glsl" };

    // events
    sig::signal<void ()> draw;

    struct key_event { int key, scancode, action, mods; };
    sig::signal<void (key_event)> key; //

    struct mouse_click_event { int button,action,mods; qvm::vec2 pos; };
    sig::signal<void (mouse_click_event)> mouse_click; //

    sig::signal<void (qvm::vec2)> mouse_move; //

    sig::signal<void (double x, double y)> mouse_scroll; // in glfw coordinates

public:

    window_c();
    ~window_c();

    qvm::ivec2 get_size() const
    {
        int width, height;
        glfwGetFramebufferSize(window.h, &width, &height);
        return {width, height};
    }

    void swap()
    {
        glfwSwapBuffers(window.h);
    }

    bool closing()
    {
        return glfwWindowShouldClose(window.h);
    }

    void close()
    {
        glfwSetWindowShouldClose(window.h, GLFW_TRUE);
    }

    void poll()
    {
        glfwPollEvents();
    }

    // from pixels to gl (-1,1)^2
    qvm::vec2 convert_from_pixel_coords(double x, double y);
    qvm::vec2 convert_from_pixel_coords_size(double x, double y);

    // from gl to pixels
    qvm::vec2 convert_to_pixel_coords(qvm::vec2 v);

    qvm::vec2 convert_to_pixel_coords_size(qvm::vec2 v);

    // win32 handle
    void * get_hwnd();

    GLFWwindow * wnd() { return window.h; }

};


#endif //ZENGINE_WINDOW_HPP
