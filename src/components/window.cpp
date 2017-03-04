//
// Created by fotyev on 2016-11-27.
//

#include "window.hpp"

#include "config.hpp"
#include "main.hpp"

window_c::window_setup::window_setup()
{
    //=- collect('config', {name=>'msaa', type=>'unsigned', def=>'16', save=>1});

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, g_app->config->msaa > 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, g_app->config->msaa);

}


//=- collect('config', {name=>'title', type=>'string', def=>'"window"', save=>1});

// SDL_WINDOWPOS_UNDEFINED
//=- collect('config', {name=>'windowpos_x', type=>'unsigned', def=>'0x1FFF0000u', save=>1});
//=- collect('config', {name=>'windowpos_y', type=>'unsigned', def=>'0x1FFF0000u', save=>1});

//=- collect('config', {name=>'resolution_x', type=>'unsigned', def=>'1280', save=>1});
//=- collect('config', {name=>'resolution_y', type=>'unsigned', def=>'720', save=>1});

// resizable
//=- collect('config', {name=>'window_flags', type=>'unsigned', def=>'0x00000020', save=>1});

window_c::window_c() : setup(),
                       window(g_app->config->title,
                              g_app->config->windowpos_x, g_app->config->windowpos_y,
                              g_app->config->resolution_x, g_app->config->resolution_y,
                              g_app->config->window_flags | SDL_WINDOW_OPENGL),
                       ctx(window.Get())

{
    //=- collect('config', {name=>'vsync', type=>'bool', def=>'true', save=>1});
    SDL_GL_SetSwapInterval(g_app->config->vsync);
}

void window_c::swap()
{
    SDL_GL_SwapWindow(window.Get());
}

