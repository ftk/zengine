//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_GAME_HPP
#define ZENGINE_GAME_HPP

//#include <SDL2/SDL.h>
#include <boost/noncopyable.hpp>
//#include <boost/container/stable_vector.hpp>
#include <memory>

// in case of Component being an interface class, you need to call load<implementation>()
template <typename Component>
class component : public std::unique_ptr<Component>
{
    typedef std::unique_ptr<Component> base;
public:
    using base::base;

    void load() { return load<Component>(); }

    template <typename Impl, typename... Args>
    void load(Args&&... args) { unload(); *this = std::make_unique<Impl>(std::forward<Args>(args)...); }

    void unload() { this->reset(); }

private:
    using base::release;
};


class application : boost::noncopyable
{

public:
    // is game running
    bool running = true;


	unsigned loop_time = 0;

    // components
public:
    /*<
     join '', map { my $class = $_->{interface} // $_->{class};
     				"\n\tcomponent<class $class> $_->{name};" } get_components;
     %*/
	component<class config_c> config;
	component<class modules_c> modules;
	component<class sdl_c> sdl_init;
	component<class window_c> window;
	component<class textures_c> textures;
	component<class fonts_c> fonts;
	component<class network_c> network;
	component<class netgame_i> netgame;
	component<class script_c> script;/*>*/

//private:
    application() noexcept;
    ~application();

public:

	void init_components();

    void run();


    void get_input();

    void draw();

};

#endif //ZENGINE_GAME_HPP
