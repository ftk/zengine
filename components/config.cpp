//
// Created by fotyev on 2016-11-06.
//

#include "components/config.hpp"
#include "main.hpp"
#include "components/modules.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
//#include <boost/archive/xml_iarchive.hpp>

#include <fstream>
#include <iostream>

#include "util/hash.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/lexical_cast.hpp>
/*< serialize_free_nvp 'config_c', sort(grep defined, map {$_->{name} if $_->{save}} dispatch('config'));; %*/
namespace boost { namespace serialization {
template <class Archive> void serialize(Archive & ar, config_c & t, const unsigned int) {
ar & make_nvp("mastersrv_ip", t.mastersrv_ip) & make_nvp("mastersrv_port", t.mastersrv_port) & make_nvp("msaa", t.msaa) & make_nvp("resolution_x", t.resolution_x) & make_nvp("resolution_y", t.resolution_y) & make_nvp("texture_cache", t.texture_cache) & make_nvp("title", t.title) & make_nvp("vsync", t.vsync) & make_nvp("window_flags", t.window_flags) & make_nvp("windowpos_x", t.windowpos_x) & make_nvp("windowpos_y", t.windowpos_y);
}}}
/*>*/


config_c::config_c()
:
/*<
    join ",\n", map {$_->{name} . '{' . ($_->{def} // '') . '}' } dispatch('config');
 %*/mastersrv_ip{"127.0.0.1"},
mastersrv_port{9999},
msaa{16},
resolution_x{1280},
resolution_y{720},
texture_cache{1024 * 1024 * 64},
title{"window"},
vsync{true},
window_flags{0x00000020},
windowpos_x{0x1FFF0000u},
windowpos_y{0x1FFF0000u}/*>*/
{
    std::ifstream ifs("config.xml");
    if(ifs)
    {
        try
        {
            using namespace boost::archive;
            xml_iarchive ia(ifs, no_tracking | no_codecvt);
            ia >> boost::serialization::make_nvp("config", *this);//(*this);
        } catch(std::exception& e)
        {
            std::cerr << "Error loading config: " << e.what() << std::endl;
        }
    }
}

config_c::~config_c()
{
    std::ofstream ofs("config.xml");
    if(ofs)
    {
        try
        {
            using namespace boost::archive;
            xml_oarchive oa(ofs, no_tracking | no_codecvt);
            oa << boost::serialization::make_nvp("config", *this);
        }
        catch(std::exception& e)
        {
            std::cerr << "Error saving config: " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "Unknown error saving config" << std::endl;
        }
    }
}

bool config_c::on_event(const SDL_Event& ev)
{
    switch(ev.type)
    {
        case SDL_KEYDOWN:
        {
            const auto mod = ev.key.keysym.mod;
            switch(ev.key.keysym.scancode)
            {

                /*<
    join "\n", map {"case SDL_SCANCODE_$_->{key}: " . (defined($_->{mod})?"if(mod & $_->{mod})":'') ."{$_->{action};} break;" } dispatch('cbind');
 %*//*>*/
                default:
                    break;
            }
        } break;
    }
    return true;
}

std::set<std::string> config_c::shader_params()
{
    std::set<std::string> params;

    //=- #collect('config', {name=>'shd_test', type=>'bool'});
    /*< join "\n", map {qq(if(shd_$_) {params.insert("$_");}) } grep s/^shd_//, map { $_->{name} } dispatch('config');
     %*//*>*/


    return params;
}

bool config_c::set(const std::string& varname, const std::string& value)
{
    try
    {
        switch(fnv1a::hash(varname))
        {
            /*< join "\n\t\t", map {qq[case "$_->{name}"_fnv: $_->{name} = boost::lexical_cast<$_->{type}>(value); return true;]} dispatch('config');
            %*/case "mastersrv_ip"_fnv: mastersrv_ip = boost::lexical_cast<string>(value); return true;
		case "mastersrv_port"_fnv: mastersrv_port = boost::lexical_cast<uint16_t>(value); return true;
		case "msaa"_fnv: msaa = boost::lexical_cast<unsigned>(value); return true;
		case "resolution_x"_fnv: resolution_x = boost::lexical_cast<unsigned>(value); return true;
		case "resolution_y"_fnv: resolution_y = boost::lexical_cast<unsigned>(value); return true;
		case "texture_cache"_fnv: texture_cache = boost::lexical_cast<unsigned>(value); return true;
		case "title"_fnv: title = boost::lexical_cast<string>(value); return true;
		case "vsync"_fnv: vsync = boost::lexical_cast<bool>(value); return true;
		case "window_flags"_fnv: window_flags = boost::lexical_cast<unsigned>(value); return true;
		case "windowpos_x"_fnv: windowpos_x = boost::lexical_cast<unsigned>(value); return true;
		case "windowpos_y"_fnv: windowpos_y = boost::lexical_cast<unsigned>(value); return true;/*>*/
            default:
                return false;
        }
    }
    catch(const boost::bad_lexical_cast& e)
    {
        LOGGER(warn, "can't set", varname, "to", value, ':', e.what());
    }
    return false;
}


/*< # simple wrappers
    sub config_bool # name, key
    {
    collect('config', {name=>$_[0], type=>'bool', def=>'false'});
    collect('cbind', {key=>$_[1], action=>"$_[0] = !$_[0]", mod=>$_[2]});
    }

 >*/
