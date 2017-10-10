//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_WINDOW_HPP
#define ZENGINE_WINDOW_HPP


//#include <SDL.h>
//#include <SDL_image.h>

//#include <SDL2pp/SDL.hh>
//#include <SDL2pp/SDLImage.hh>
#include <SDL2pp/Window.hh>
//#include <SDL2pp/Renderer.hh>
//#include <SDL2pp/Texture.hh>
//#include <SDL2pp/Surface.hh>

//#include "util/optional.hpp"
#include "util/geometry.hpp"

#include "opengl/opengl.hpp"
#include "opengl/render2d.hpp"
#include "opengl/math.hpp"


//=- register_component(class=>'window_c', name=>'window', priority=>10);

// opengl window
class window_c
{
    struct window_setup
    {
        window_setup();
    } setup;
    SDL2pp::Window window;
    gl ctx;
public:
    renderer_2d render;

public:
    window_c();


    Point get_size() const
    {
        return window.GetDrawableSize(); // window_->GetDrawableSize();
    }

    double get_aspect_ratio() const
    {
        Point size = get_size();
        return double(size.x) / size.y;
    }

    void swap();

    qvm::vec2 to_gl_coords(Point p) const
    {
        Point s = get_size();
        return qvm::vec2{2.f * p.x / s.x - 1.f, -2.f * p.y / s.y + 1.f};
    }

};

// sdl window
#if 0
struct window_c
{
    optional<SDL2pp::Window> window_;
    optional<SDL2pp::Renderer> render_;


    void create_window(unsigned window_flags = SDL_WINDOW_RESIZABLE,
                       optional<unsigned> renderer_flags = SDL_RENDERER_ACCELERATED)
    {
        window_ = SDL2pp::Window{"window",
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                1280, 720,
                                window_flags};
        if(renderer_flags)
        {
            render_ = SDL2pp::Renderer{*window_, -1, *renderer_flags};
        }
    }

    SDL2pp::Renderer& render()
    {
        return *render_;
    }

    Point get_size() const
    {
        return render_ ?
        render_->GetOutputSize()
        : window_->GetDrawableSize(); // window_->GetDrawableSize();
    }

    double get_aspect_ratio() const
    {
        Point size = get_size();
        return double(size.x) / size.y;
    }

};
#endif

#endif //ZENGINE_WINDOW_HPP
