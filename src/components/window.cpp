//
// Created by fotyev on 2016-11-27.
//

#include "window.hpp"

#include "config.hpp"
#include "main.hpp"

window_c::window_setup::window_setup()
{
    int msaa = g_app->config->get("window.msaa", 16);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa > 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa);

}


window_c::window_c() : setup(),
                       window(g_app->config->get("window.title", "window"),
                              g_app->config->get("window.x", SDL_WINDOWPOS_UNDEFINED),
                              g_app->config->get("window.y", SDL_WINDOWPOS_UNDEFINED),
                              g_app->config->get("window.width", 1280),
                              g_app->config->get("window.height", 720),
                              g_app->config->get("window.flags", (int)SDL_WINDOW_RESIZABLE) | SDL_WINDOW_OPENGL
                              ),
                       ctx(window.Get())

{
    SDL_GL_SetSwapInterval(g_app->config->get("window.vsync", true));
}

void window_c::swap()
{
    SDL_GL_SwapWindow(window.Get());
}

