//
// Created by fotyev on 2016-10-16.
//

#include "application.hpp"

#if 0 /*%
 join "\n", map { qq(#include "$_") } dispatch_s('component_headers') ;
 #%*/
#else
#include "src/components/config.hpp"
#include "src/components/glfw.hpp"
#include "src/components/mixer.hpp"
#include "src/components/netgame_impl.hpp"
#include "src/components/network.hpp"
#include "src/components/resources.hpp"
#include "src/components/window.hpp"
#endif

#include "opengl/opengl.hpp"
#include "util/log.hpp"

void application::run()
{
    while(running)
    {
        // frame limiter loop

        window->poll();
        if(window->closing())
            running = false;

        draw();
    }
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
#define LOAD_COMPONENT(name,classname) try {if(!name) {name.load<classname>();}} \
        catch(...){logger(log_level::fatal,"unable to load component", #name);throw;}
        /*<
         join '', map { "\n\t\tLOAD_COMPONENT($_->{name}, $_->{class})" } grep { !defined $_->{defer} } get_components ;
         %*/
		LOAD_COMPONENT(config, config_c)
		LOAD_COMPONENT(glfw_init, glfw_c)
		LOAD_COMPONENT(window, window_c)
		LOAD_COMPONENT(resources, resources_c)
		LOAD_COMPONENT(mixer, mixer_c)
		LOAD_COMPONENT(network, network_c)
		LOAD_COMPONENT(netgame, netgame_c)/*>*/
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

