//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_GAME_HPP
#define ZENGINE_GAME_HPP

//#include <SDL.h>
#include <boost/noncopyable.hpp>
//#include <boost/container/stable_vector.hpp>
#include <memory>

// in case of Component being an interface class, you need to call load<implementation>()
template <typename Component>
class app_component : public std::unique_ptr<Component>
{
    typedef std::unique_ptr<Component> base;
public:
    using base::base;

    void load() { return load<Component>(); }

    template <typename Impl, typename... Args>
    void load(Args&&... args) { unload(); base::operator=(std::make_unique<Impl>(std::forward<Args>(args)...)); }

    void unload() { this->reset(); }

private:
    using base::release;
};


class application : boost::noncopyable
{

public:
    // is game running
    bool running = true;

    // components
public:
    /*<
     join '', map { my $class = $_->{interface} // $_->{class};
     				"\n\tapp_component<class $class> $_->{name};" } get_components;
     %*/
	app_component<class config_c> config;
	app_component<class glfw_c> glfw_init;
	app_component<class window_c> window;
	app_component<class resources_c> resources;
	app_component<class network_c> network;
	app_component<class netgame_i> netgame;/*>*/

//private:
    application() noexcept;
    ~application();

public:

	void init_components();

    void run();

	void draw();

};

#endif //ZENGINE_GAME_HPP
