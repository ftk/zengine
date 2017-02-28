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
#include <opengl/render2d.hpp>

#include <SDL2/SDL_pixels.h>

/*< register_module(class=>'fps', name=>'fps'); >*/

class fps : public basic_module
{


    void draw() override
    {
        int loop_time = g_app->loop_time;
        auto& texture = g_app->textures->get(loop_time, [=]() {
            return g_app->fonts->def.RenderText_Blended(boost::lexical_cast<std::string>(loop_time), SDL_Color{255,255,255,255}).Convert(SDL_PIXELFORMAT_RGBA32);});

        //auto& texture = g_app->textures->get_from_file(const_string("resources/test.png"));
        //texture.set_params(GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);

        gl::Enable(GL_BLEND);
        gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        g_app->window->render.set4dpos();
        g_app->window->render.copy({-1.f, 0.95f}, {-0.95f, 1.f}, nullopt, texture);


    }



};



#endif //ZENGINE_FPS_HPP
