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

#include <SDL_timer.h>


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
        draw();

        ticks = SDL_GetTicks();
        auto loop_time = ticks - prev_ticks;


        if(loop_time < 16) // limit to 60 fps for now (16 = 1000/60)
        {
            SDL_Delay(16 - (loop_time));
        }
        prev_ticks = SDL_GetTicks();

    }

    modules->clear();
    textures->clear();
}

void application::draw()
{
    gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // signal
    modules->draw();

    window->swap();
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
#define LOAD_COMPONENT(name,classname) try {if(!name) {name.load<classname>();}} \
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

