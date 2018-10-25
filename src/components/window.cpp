//
// Created by fotyev on 2016-11-27.
//

#include "window.hpp"

#include "config.hpp"
#include "main.hpp"

#include <GLFW/glfw3.h>

#include "util/log.hpp"
#include "opengl/opengl.hpp"

window_c::window_handle::window_handle(int width, int height, const char * title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLES_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);


#if GL_DEBUG >= 2
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int msaa = g_app->config->get("window.msaa", 16);
    glfwWindowHint(GLFW_SAMPLES, msaa);

    LOGGER(info, "creating window", width, height, title);
    h = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if(!h)
        throw std::runtime_error{"Cant create window!"};

    if(auto x = g_app->config->get_optional<int>("window.x"))
        if(auto y = g_app->config->get_optional<int>("window.y"))
            glfwSetWindowPos(h, *x, *y);

    glfwMakeContextCurrent(h);

    glfwSwapInterval(g_app->config->get<int>("window.vsync", 1));

    if(auto x = g_app->config->get_optional<int>("window.ratio.x"))
        if(auto y = g_app->config->get_optional<int>("window.ratio.y"))
            glfwSetWindowAspectRatio(h, *x, *y);


}

window_c::window_handle::~window_handle()
{
    if(h)
        glfwDestroyWindow(h);
}

#ifdef SINGLE_WINDOW
static window_c * window;
#define GET_WINDOW(w) (window)
#else
#include <unordered_map>
static std::unordered_map<GLFWwindow *, window_c *> windows;
#define GET_WINDOW(w) (assume(windows.count(w)),windows[w])
#endif

static void key_cb(GLFWwindow * w, int key, int scancode, int action, int mods)
{
    GET_WINDOW(w)->key({key, scancode, action, mods});
}

static void mouse_click_cb(GLFWwindow * w, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    GET_WINDOW(w)->mouse_click({button, action, mods, GET_WINDOW(w)->convert_from_pixel_coords(x,y)});
}

static void mouse_move_cb(GLFWwindow * w, double x, double y)
{
    GET_WINDOW(w)->mouse_move(GET_WINDOW(w)->convert_from_pixel_coords(x,y));
}

static void mouse_scroll_cb(GLFWwindow * w, double x, double y)
{
    GET_WINDOW(w)->mouse_scroll(x, y);
}

window_c::window_c() : window(
                              g_app->config->get("window.width", 1280),
                              g_app->config->get("window.height", 720),
                              g_app->config->get("window.title", "window").c_str()
                              ),
                       ctx()

{
#ifdef SINGLE_WINDOW
    assume(window == nullptr);
    window = this;
#else
    windows.reserve(1);
    windows[window.h] = this;
#endif
    glfwSetKeyCallback(window.h, &key_cb);
    glfwSetMouseButtonCallback(window.h, &mouse_click_cb);
    glfwSetCursorPosCallback(window.h, &mouse_move_cb);
    glfwSetScrollCallback(window.h, &mouse_scroll_cb);

    glfwSetFramebufferSizeCallback(window.h, [](GLFWwindow* window, int width, int height) {
        gl::Viewport(0, 0, width, height);
    });

    auto size = get_size();
    gl::Viewport(0, 0, qvm::X(size), qvm::Y(size));

}



window_c::~window_c()
{
#ifdef SINGLE_WINDOW
    assume(window == this);
    window = nullptr;
#else
    windows.erase(windows.find(window.h));
#endif
}

// [0,w)x[0,h) -> [-1,1]^2
qvm::vec2 window_c::convert_from_pixel_coords(double x, double y)
{
    using namespace qvm;
    auto size = get_size();
    return vec2{-1.f + 2.f * float(x) / X(size), 1.f - 2.f * float(y) / Y(size)};
}

qvm::vec2 window_c::convert_from_pixel_coords_size(double x, double y)
{
    using namespace qvm;
    auto size = get_size();
    return vec2{2.f * float(x) / X(size), 2.f * float(y) / Y(size)};
}

qvm::vec2 window_c::convert_to_pixel_coords(qvm::vec2 v)
{
    using namespace qvm;
    auto size = get_size();
    return vec2{X(size) * ((X(v) + 1.f) / 2.f), Y(size) * ((-Y(v) + 1.f) / 2.f)};
}

qvm::vec2 window_c::convert_to_pixel_coords_size(qvm::vec2 v)
{
    using namespace qvm;
    auto size = get_size();
    return vec2{X(size) * X(v) / 2.f, Y(size) * Y(v) / 2.f};
}

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#endif

void * window_c::get_hwnd()
{
#ifdef _WIN32
    return (void *)glfwGetWin32Window(window.h);
#else
    return nullptr;
#endif
}



#include "opengl/textrender.hpp"
#include "components/resources.hpp"
void window_c::render_text_box(qvm::vec2 ll, qvm::vec2 size, string_view text, unsigned int lines)
{
    using namespace qvm;
    vec2 pixsize = convert_to_pixel_coords_size(size);
    X(pixsize) = ceil(X(pixsize));
    Y(pixsize) = ceil(Y(pixsize));
    size = convert_from_pixel_coords_size(X(pixsize), Y(pixsize));
    texture& tex = g_app->resources->textures.get(text, [lines, pixsize](string_view text){
        return make_text_box(g_app->resources->default_font, text, pixsize, lines);
    });
    //tex.bind();
    //gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    text_render.copy(tex, ll, size);
}
