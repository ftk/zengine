//
// Created by fotyev on 2016-10-16.
//

#include "application.hpp"

#if 0 /*%
 join "\n", map { qq(#include "$_") } dispatch_s('component_headers') ;
 #%*/
#else
#include "src/components/collections.hpp"
#include "src/components/config.hpp"
#include "src/components/modules.hpp"
#include "src/components/netgame_impl.hpp"
#include "src/components/network.hpp"
#include "src/components/script.hpp"
#include "src/components/sdl.hpp"
#include "src/components/window.hpp"
#endif

#include <iostream>
#include <components/config.hpp>


#include "opengl/opengl.hpp"

#include "util/log.hpp"

void application::run()
{



    modules->init();


    auto prev_ticks = SDL_GetTicks(), ticks = SDL_GetTicks();

    while(running)
    {
// frame limiter loop

        get_input();
        // update states
        draw();

        /*constexpr string_view str = const_string(
                u8R"(In cryptography, the avalanche effect is the desirable property of cryptographic algorithms, typically block ciphers and cryptographic hash functions wherein if when an input is changed slightly (for example, flipping a single bit) the output changes significantly (e.g., half the output bits flip). In the case of high-quality block ciphers, such a small change in either the key or the plaintext should cause a drastic change in the ciphertext. The actual term was first used by Horst Feistel,[1] although the concept dates back to at least Shannon's diffusion.)"
        );
        //boost::lexical_cast<std::string>(textures->get_VRAM_used());

        int words = std::min(SDL_GetTicks() / 50, 100u);
        auto& texture = textures->get(fnv1a::foldr(str, words, loop), [=]() {
            return fonts->render_multiline(fonts->def, boost::lexical_cast<std::string>(loop)+str.to_string(),
                                        Point(400,800), SDL_Color{255,255,255,255}, words);});

        window->render().Copy(texture, SDL2pp::NullOpt, Point(0,0));
        */

        ticks = SDL_GetTicks();
        loop_time = ticks - prev_ticks;


        if(loop_time < 16) // limit to 60 fps for now (16 = 1000/60)
            SDL_Delay(16 - (loop_time));
        prev_ticks = SDL_GetTicks();

        /*
        static unsigned print = SDL_GetTicks();
        if(ticks >= print + 5000)
        {
            std::cout << loop_time << '\n';
            print = ticks;
        }
         */

    }

    modules->clear();
    textures->clear();
}

void application::draw()
{
    //window->render().Clear();

    gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // signal
    modules->draw();


    window->swap();
    //window->render().Present();

}


void application::get_input()
{

        SDL_Event ev;
        while(SDL_PollEvent(&ev))
        {
            if(!modules->on_event(ev))
                continue;

            switch(ev.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if(ev.key.keysym.sym == SDLK_ESCAPE)
                        running = false;
                    break;
                case SDL_WINDOWEVENT:
                {
                    if(ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        //std::cout << ev.window.data1 << ' ' << ev.window.data2 << '\n';
                        gl::Viewport(0, 0, ev.window.data1, ev.window.data2);
                    }
                }break;
            }
        }

}

void application::init_components()
{
    try
    {
#define LOAD_COMPONENT(name,classname) try {name.load<classname>();} \
        catch(...){logger(log_level::fatal,"unable to load component", #name);throw;}
        /*<
         join '', map { "\n\t\tLOAD_COMPONENT($_->{name}, $_->{class})" } grep { !defined $_->{defer} } get_components ;
         %*/
		LOAD_COMPONENT(config, config_c)
		LOAD_COMPONENT(modules, modules_c)
		LOAD_COMPONENT(sdl_init, sdl_c)
		LOAD_COMPONENT(window, window_c)
		LOAD_COMPONENT(textures, textures_c)
		LOAD_COMPONENT(fonts, fonts_c)
		LOAD_COMPONENT(network, network_c)
		LOAD_COMPONENT(netgame, netgame_c)
		LOAD_COMPONENT(script, script_c)/*>*/
#undef LOAD_COMPONENT
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception on initializing: " << e.what() << std::endl;
        throw;
    }
}

application::application() noexcept
{

}


// empty destructor to generate unique_ptr deleters for incomplete types
application::~application()
{
}

