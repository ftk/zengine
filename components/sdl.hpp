//
// Created by fotyev on 2016-11-27.
//

#ifndef ZENGINE_SDL_HPP
#define ZENGINE_SDL_HPP


#include <SDL2pp/SDLImage.hh>
#include <SDL2pp/SDLTTF.hh>
#include <SDL2pp/SDLMixer.hh>
#include <SDL2pp/SDL.hh>


#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//=- register_component(class=>'sdl_c', name=>'sdl_init', priority=>5);
class sdl_c
{
    // initialize SDL
    SDL2pp::SDL sdl{SDL_INIT_VIDEO | SDL_INIT_AUDIO};
    SDL2pp::SDLImage image{IMG_INIT_PNG};
    SDL2pp::SDLTTF ttf{};
    //SDL2pp::SDLMixer mixer{MIX_INIT_OGG|MIX_INIT_MP3};
};

#endif //ZENGINE_SDL_HPP
