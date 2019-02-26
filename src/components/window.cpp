//
// Created by fotyev on 2016-11-27.
//

#include "window.hpp"

#include "main.hpp"

#include <GLFW/glfw3.h>

#include "util/semiconfig.hpp"
#include "util/log.hpp"
#include "opengl/opengl.hpp"

window_c::window_handle::window_handle(int width, int height, const char * title)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLES_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, SCFG(window.egl, 0) ? GLFW_EGL_CONTEXT_API : GLFW_NATIVE_CONTEXT_API);


#if GL_DEBUG >= 2
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int msaa = SCFG(window.msaa, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_SAMPLES, msaa);

    LOGGER(info, "creating window", width, height, title);
    h = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if(!h)
        throw std::runtime_error{"Cant create window!"};

    if(auto x = static_config::at_optional<int>(ID("window.x")))
        if(auto y = static_config::at_optional<int>(ID("window.y")))
            glfwSetWindowPos(h, *x, *y);

    glfwMakeContextCurrent(h);

    glfwSwapInterval(SCFG(window.vsync, 1));

    if(auto x = static_config::at_optional<int>(ID("window.ratio.x")))
        if(auto y = static_config::at_optional<int>(ID("window.ratio.y")))
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
#define GET_WINDOW(w) reinterpret_cast<window_c *>(glfwGetWindowUserPointer(w))
#endif


window_c::window_c() : window(
                              SCFG(window.width, 1280),
                              SCFG(window.height, 720),
                              SCFG(window.title, "window").c_str()
                              ),
                       ctx()

{
#ifdef SINGLE_WINDOW
    assume(window == nullptr);
    window = this;
#else
    glfwSetWindowUserPointer(window.h, this);
#endif
    glfwSetKeyCallback(window.h, [](GLFWwindow * w, int key, int scancode, int action, int mods) {
        GET_WINDOW(w)->key({key, scancode, action, mods});
    });

    glfwSetMouseButtonCallback(window.h, [](GLFWwindow * w, int button, int action, int mods) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        GET_WINDOW(w)->mouse_click({button, action, mods, GET_WINDOW(w)->convert_from_pixel_coords(x,y)});
    });

    glfwSetCursorPosCallback(window.h, [](GLFWwindow * w, double x, double y) {
        auto& wnd = *GET_WINDOW(w);
        qvm::vec2 newpos = wnd.convert_from_pixel_coords(x,y);
        //wnd.mouse_move(newpos);
        //wnd.mouse_move_delta(newpos - wnd.cursor_pos);
        wnd.cursor_pos = newpos;
    });

    glfwSetScrollCallback(window.h, [](GLFWwindow * w, double x, double y) {
        GET_WINDOW(w)->mouse_scroll(x, y);
    });

    glfwSetFramebufferSizeCallback(window.h, [](GLFWwindow* w, int width, int height) {
        GET_WINDOW(w)->resize(width, height);
    });

    resize.connect([](int w, int h) { gl::Viewport(0, 0, w, h); }); // TODO: move

    auto size = get_size();
    resize(qvm::X(size), qvm::Y(size));

    draw.connect([this]() {
        if(old_cursor_pos != cursor_pos)
        {
            mouse_move(cursor_pos);
            mouse_move_delta(cursor_pos - old_cursor_pos);
            old_cursor_pos = cursor_pos;
        }
    });

}



window_c::~window_c()
{
#ifdef SINGLE_WINDOW
    assume(window == this);
    window = nullptr;
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

void window_c::toggle_cursor(bool toggle)
{
    glfwSetInputMode(window.h, GLFW_CURSOR, toggle ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}


#include "opengl/textrender.hpp"
#include "components/resources.hpp"
void window_c::render_text_box(qvm::vec2 ll, qvm::vec2 size, string_view text, unsigned int lines)
{
    using namespace qvm;
    vec2 pixsize = convert_to_pixel_coords_size(size);
    X(pixsize) = ceil(X(pixsize));
    Y(pixsize) = ceil(Y(pixsize));
    if(X(pixsize) <= 0 || Y(pixsize) <= 0)
        return; // invalid size
    size = convert_from_pixel_coords_size(X(pixsize), Y(pixsize));
    texture& tex = g_app->resources->textures.get(text, [lines, pixsize](string_view text){
        return make_text_box(g_app->resources->default_font, text, pixsize, lines);
    });
    //tex.bind();
    //gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    text_render.copy(tex, ll, size);
}
