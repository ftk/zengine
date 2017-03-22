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

#include <ctime>


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

        auto& texture = g_app->textures->get(fps,
                                             g_app->fonts->lazy_render(boost::lexical_cast<std::string>(fps), SDL_Color{255,255,255,255}));

        gl::Enable(GL_BLEND);
        gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //auto t = g_app->window->render.transform(renderer_2d::transform_t::rect({-1.f, 0.95f}, {0.05, 0.05}));
        //renderer_2d::transform_t t{g_app->window->render, renderer_2d::transform_t::rect({-1.f, 0.95f}, {0.05, 0.05})};
        g_app->window->render.copy2(texture, {-1.f, 0.95f}, {0.05, 0.05});

    }

};



#endif //ZENGINE_FPS_HPP
