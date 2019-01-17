//
// Created by fotyev on 2016-10-16.
//

#include "application.hpp"

#if 0 /*%
 join "\n", map { qq(#include "$_->{header}") } get_components ;
 #%*/
#else
#include "src/components/config.hpp"
#include "src/components/glfw.hpp"
#include "src/components/window.hpp"
#include "src/components/joystick.hpp"
#include "src/components/input.hpp"
#include "src/components/resources.hpp"
#include "src/components/network.hpp"
#include "src/components/netgame_impl.hpp"
#include "src/components/script.hpp"
#endif

#include "opengl/opengl.hpp"
#include "util/log.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
static void app_loop();
#endif

void application::run()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(&app_loop, 0, 1);
#else
    while(running)
    {
        // frame limiter loop

        window->poll();
        if(window->closing())
            running = false;

        draw();
    }
#endif
}

void application::draw()
{
    gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // signal
    window->draw();

    window->swap();
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
		LOAD_COMPONENT(glfw_init, glfw_c)
		LOAD_COMPONENT(window, window_c)
		LOAD_COMPONENT(joystick, joystick_c)
		LOAD_COMPONENT(input, input_map_c)
		LOAD_COMPONENT(resources, resources_c)
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

#ifdef __EMSCRIPTEN__
#include "main.hpp"
void app_loop()
{
    g_app->window->poll();
    g_app->draw();
}
#endif

