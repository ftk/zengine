//
// Created by fotyev on 2016-10-16.
//

#ifndef ZENGINE_FPS_HPP
#define ZENGINE_FPS_HPP

#include "basic_module.hpp"



#include "util/geometry.hpp"

#include "main.hpp"
#include "components/window.hpp"
#include "components/collections.hpp"

#include <boost/lexical_cast.hpp>
#include "opengl/render2d.hpp"

#include <SDL_pixels.h>
#include <ctime>

#include "util/sdl_workaround.hpp"

/*< register_module(class=>'fps', name=>'fps'); >*/

class fps : public basic_module
{
    unsigned fps = 0, frames;
    void draw() override
    {
        static auto next_time = time(nullptr) + 1;
        ++frames;
        if(time(nullptr) >= next_time)
        {
            fps = frames;
            frames = 0;
            next_time = time(nullptr) + 1;
        }

        auto& texture = g_app->textures->get(fnv1a::stdhash(fps),
                                             g_app->fonts->lazy_render(boost::lexical_cast<std::string>(fps), SDL_Color{255,255,255,255}));

        gl::Enable(GL_BLEND);
        gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_app->window->render.set4dpos();
        g_app->window->render.copy({-1.f, 0.95f}, {-0.95f, 1.f}, nullopt, texture);

    }

};



#endif //ZENGINE_FPS_HPP
