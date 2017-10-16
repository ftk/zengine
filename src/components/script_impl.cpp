//
// Created by fotyev on 2017-01-28.
//

#ifndef NO_SCRIPTING

#include "script.hpp"
//#include <chaiscript/dispatchkit/dispatchkit.hpp>
//#include <chaiscript/dispatchkit/register_function.hpp>
#include <chaiscript/chaiscript_basic.hpp>
#include "util/assert.hpp"

#include "components/config.hpp"
#include "components/modules.hpp"

/*<
        join "", map { qq(\n#include "$_"\n) } (dispatch_s('module_headers'));#, dispatch('component_headers')) .  ;
    %*/
#include "examples/zwarudo/controller.hpp"

#include "src/modules/controls.hpp"

#include "src/modules/fps.hpp"

#include "src/modules/optionbox.hpp"
/*>*/


using namespace chaiscript;
ModulePtr script_register_bindings(ModulePtr m)
{
    auto& chai = *m;


    auto modules = g_app->modules.get();

    chai.add(user_type<basic_module>(), "basic_module");
    chai.add(fun([](int m) {SDL_SetRelativeMouseMode(m ? SDL_TRUE : SDL_FALSE);}), "SDL_SetRelativeMouseMode");
/*
	chai.add(user_type<voxel_ctx>(), "Vpos");
	chai.add(constructor<voxel_ctx ()>(), "Vpos");
	chai.add(constructor<voxel_ctx (const voxel_ctx&)>(), "Vpos");
	chai.add(fun([](const voxel_ctx& p) -> int { return p.pos[0];}), "get_x");
	chai.add(fun([](const voxel_ctx& p) -> int { return p.pos[1];}), "get_y");
	chai.add(fun([](const voxel_ctx& p) -> int { return p.pos[2];}), "get_z");
	chai.add(fun([](int x, int y, int z) -> voxel_ctx { return voxel_ctx{x,y,z};}), "Vpos");
	///chai.add(fun(voxel_ctx::pos[1]), "y");
	///chai.add(fun(voxel_ctx::pos[2]), "z");
*/

/*<
    my $s='';

    # modules
    for my $module (dispatch('modules')) {
    my $name = $module->{class};
    my $sname = $name;# ~ s/::/_/g;
    #add class
    $s .= qq[\n\tchai.add(user_type<$name>(), "\u$sname");];

    if (!defined($module->{unmanaged})) {
    	$s .= qq[\n\tchai.add(base_class<basic_module, $name>());];
    	#add constructors
    	my $ctors = $module->{ctors} // [''];
    	$s .= qq[\n\tchai.add(fun(&modules_c::load<$name] . ($_ ? ", $_" : '') . qq[>, modules), "load_${sname}");] for (@$ctors);
	}

    #add functions
    for (@{$module->{scriptexport}}) {
        $s .= qq[\n\tchai.add(fun(&$name::$_), "$_");];
    }


    }

    # components
    for my $c (dispatch('components')) {
    my $name = $c->{interface} // $c->{class};
    $s .= qq[\n\tchai.add(fun(&$name::$_, g_app->$c->{name}.get()), "$c->{name}_${_}");] for (@{$c->{scriptexport}})
    }


    $s;
%*/
	chai.add(user_type<controller>(), "Controller");
	chai.add(base_class<basic_module, controller>());
	chai.add(fun(&modules_c::load<controller>, modules), "load_controller");
	chai.add(fun(&controller::join), "join");
	chai.add(fun(&controller::host), "host");
	chai.add(fun(&controller::stop), "stop");
	chai.add(fun(&controller::send_event), "send_event");
	chai.add(fun(&controller::playing), "playing");
	chai.add(user_type<controls>(), "Controls");
	chai.add(base_class<basic_module, controls>());
	chai.add(fun(&modules_c::load<controls>, modules), "load_controls");
	chai.add(fun(&controls::set_key_handler), "set_key_handler");
	chai.add(fun(&controls::set_mouse_handler), "set_mouse_handler");
	chai.add(fun(&controls::unset_key_handler), "unset_key_handler");
	chai.add(fun(&controls::unset_mouse_handler), "unset_mouse_handler");
	chai.add(user_type<fps>(), "Fps");
	chai.add(base_class<basic_module, fps>());
	chai.add(fun(&modules_c::load<fps>, modules), "load_fps");
	chai.add(user_type<optionbox>(), "Optionbox");
	chai.add(base_class<basic_module, optionbox>());
	chai.add(fun(&modules_c::load<optionbox>, modules), "load_optionbox");
	chai.add(fun(&modules_c::load<optionbox, float,float>, modules), "load_optionbox");
	chai.add(fun(&optionbox::add), "add");
	chai.add(fun(&optionbox::clear), "clear");
	chai.add(fun(&optionbox::update), "update");
	chai.add(fun(&optionbox::set_offset), "set_offset");
	chai.add(fun(&config_c::get_param, g_app->config.get()), "config_get_param");
	chai.add(fun(&config_c::set_param, g_app->config.get()), "config_set_param");
	chai.add(fun(&modules_c::get, g_app->modules.get()), "modules_get");
	chai.add(fun(&modules_c::loaded, g_app->modules.get()), "modules_loaded");
	chai.add(fun(&modules_c::unload, g_app->modules.get()), "modules_unload");
	chai.add(fun(&netgame_i::id, g_app->netgame.get()), "netgame_id");
	chai.add(fun(&netgame_i::nodes_list, g_app->netgame.get()), "netgame_nodes_list");/*>*/


    chai.add(user_type<event_t>(), "Event");

    /*< my $s;
       # events
       for my $event (dispatch('events')) {
       my $ev = $event->{name};
       my $pt = join ', ', map { "$_->[0] $_->[1]" } @{$event->{params} // []};
       my $pa = join ', ', map { "std::move($_->[1])" } @{$event->{params} // []};
       $s .= qq[\n\tchai.add(fun([]($pt) -> event_t {return event::${ev}{${pa}};}), "event_$ev");];
       }
       $s;
      %*/
	chai.add(fun([](float x, float y) -> event_t {return event::click{std::move(x), std::move(y)};}), "event_click");
	chai.add(fun([]() -> event_t {return event::join{};}), "event_join");
	chai.add(fun([](tick_t tick, net_node_id id) -> event_t {return event::joined{std::move(tick), std::move(id)};}), "event_joined");
	chai.add(fun([](int32_t x, int32_t y, uint8_t down) -> event_t {return event::movement{std::move(x), std::move(y), std::move(down)};}), "event_movement");
	chai.add(fun([]() -> event_t {return event::node_connect{};}), "event_node_connect");
	chai.add(fun([]() -> event_t {return event::node_disconnect{};}), "event_node_disconnect");
	chai.add(fun([]() -> event_t {return event::null{};}), "event_null");
	chai.add(fun([](std::vector<net_node_id> arr) -> event_t {return event::peers{std::move(arr)};}), "event_peers");
	chai.add(fun([]() -> event_t {return event::player_join{};}), "event_player_join");
	chai.add(fun([]() -> event_t {return event::player_leave{};}), "event_player_leave");
	chai.add(fun([](std::string state) -> event_t {return event::statesync{std::move(state)};}), "event_statesync");/*>*/

    return m;
}

#endif
