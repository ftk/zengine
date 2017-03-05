//
// Created by fotyev on 2016-11-06.
//

#ifndef ZENGINE_CONFIG_HPP
#define ZENGINE_CONFIG_HPP

#include "modules/basic_module.hpp"
#include <string>
#include <set>

#include "util/assert.hpp"

//#include <boost/serialization/nvp.hpp>

#include <boost/noncopyable.hpp>


//=- register_component(class=>'config_c', name=>'config', priority=>0, scriptexport=>[qw(get set)]);
class config_c : boost::noncopyable
{

    using string = std::string;
public:


    /*<
        join "\n\t", map {$_->{type} . ' ' . $_->{name} . ';'} dispatch('config');
     %*/string bind_ip;
	uint16_t bind_port;
	string mastersrv_ip;
	uint16_t mastersrv_port;
	unsigned msaa;
	unsigned resolution_x;
	unsigned resolution_y;
	unsigned texture_cache;
	string title;
	bool vsync;
	unsigned window_flags;
	unsigned windowpos_x;
	unsigned windowpos_y;/*>*/

public:
    config_c();
    ~config_c();

    bool on_event(const SDL_Event& ev);

    bool set(const std::string& varname, const std::string& value);
	std::string get(const std::string& varname);

    std::set<std::string> shader_params();


    /*< #serialize dispatch('config_save'); %*//*>*/
};

#endif //ZENGINE_CONFIG_HPP
