//
// Created by fotyev on 2017-01-28.
//

#ifndef NO_SCRIPTING
#include "main.hpp"
#include "script.hpp"
//#include <chaiscript/dispatchkit/dispatchkit.hpp>
//#include <chaiscript/dispatchkit/register_function.hpp>
#include <chaiscript/chaiscript_basic.hpp>
#include "util/assert.hpp"

/*<
        join "", map { qq(\n#include "$_->{header}") } grep {defined($_->{scriptexport})} (dispatch('modules'), get_components);
    %*/
#include "src/components/menu.hpp"
#include "src/components/config.hpp"
#include "src/components/input.hpp"
#include "src/components/netgame_impl.hpp"/*>*/


using namespace chaiscript;
ModulePtr script_register_bindings(ModulePtr m)
{
    auto& chai = *m;

/*<
    my $s='';

    # modules
    for my $module (dispatch('modules')) {
    my $name = $module->{class};
    my $sname = $name;# ~ s/::/_/g;
    #add class
    $s .= qq[\n\tchai.add(user_type<$name>(), "\u$sname");];

    if (!defined($module->{unmanaged})) {
    	#add constructors
    	my $ctors = $module->{ctors} // [''];
    	if(!defined($module->{pointer})) {
    	$s .= qq[\n\tchai.add(constructor<${name} (${_})>(), "\u${sname}");] for (@$ctors);
    	} else {
    	 $s .= qq[\n\tchai.add(fun([](${_}) {return std::make_shared<$name>(] .
    	 join(', ', map { s/^.*\s+(.+)$/$1/; "std::move($_)"; } split(m%,\s*%, $_)) . qq[);}), "\u${sname}");] for (@$ctors);
    	 #$s .= qq[\n\tchai.add(fun<std::shared_ptr<$name> ($_)>([](auto... args) {return std::make_shared<$name>(args...);}), "\u${sname}");] for (@$ctors);
    	 $s .= qq[\n\tchai.add(fun([](std::shared_ptr<$name>& ptr) {ptr.reset();}), "reset");];
    	}
	}

    #add functions
    for (@{$module->{scriptexport}}) {
        $s .= qq[\n\tchai.add(fun(&${name}::$_), "$_");];
    }


    }

    # components
    for my $c (dispatch('components')) {
    my $cl = $c->{interface} // $c->{class};
    my $name = $c->{name};
    $s .= qq[\n\tchai.add(fun(&${cl}::$_, g_app->${name}.get()), "${name}_${_}");] for (@{$c->{scriptexport}});
    }


    $s;
%*/
	chai.add(user_type<menu_c>(), "Menu_c");
	chai.add(fun([](std::string s, menu_c::callback_t cb, qvm::vec2 pos, float pixelh) {return std::make_shared<menu_c>(std::move(s), std::move(cb), std::move(pos), std::move(pixelh));}), "Menu_c");
	chai.add(fun([](std::shared_ptr<menu_c>& ptr) {ptr.reset();}), "reset");
	chai.add(fun(&menu_c::lock), "lock");
	chai.add(fun(&config_c::get, g_app->config.get()), "config_get");
	chai.add(fun(&config_c::set, g_app->config.get()), "config_set");
	chai.add(fun(&netgame_i::id, g_app->netgame.get()), "netgame_id");
	chai.add(fun(&netgame_i::nodes_list, g_app->netgame.get()), "netgame_nodes_list");/*>*/

	chai.add(fun([](sig::signal<void (bool)>& signal, const std::function<void(bool)>& f){signal.connect(f);}), "connect");

    //chai.add(user_type<event_t>(), "Event");

    /* my $s;
       # events
       for my $event (dispatch('events')) {
       my $ev = $event->{name};
       my $pt = join ', ', map { "$_->[0] $_->[1]" } @{$event->{params} // []};
       my $pa = join ', ', map { "std::move($_->[1])" } @{$event->{params} // []};
       $s .= qq[\n\tchai.add(fun([]($pt) -> event_t {return event::${ev}{${pa}};}), "event_$ev");];
       }
       $s;
      %*//*>*/

    return m;
}

#endif
