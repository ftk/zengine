//
// Created by fotyev on 2016-11-06.
//

#ifndef ZENGINE_CONFIG_HPP
#define ZENGINE_CONFIG_HPP

#include "modules/basic_module.hpp"
#include <string>
#include <set>

#include <boost/property_tree/ptree.hpp>

//#include <vector>
#include "util/hash.hpp"
#include "util/assert.hpp"

//#include <boost/serialization/nvp.hpp>

#include <boost/noncopyable.hpp>


//=- register_component(class=>'config_c', name=>'config', priority=>0, scriptexport=>[qw(get set)]);
class config_c : boost::noncopyable
{

public:
	boost::property_tree::ptree tree;

	std::string configfile = "config.xml";
public:

	void load_from_file(string_view filename) noexcept;
	void save_to_file(string_view filename) noexcept;
    config_c();
    ~config_c();

	bool set(const std::string& varname, const std::string& value);
	std::string get(const std::string& varname);


    /*< #serialize dispatch('config_save'); %*//*>*/
};

#endif //ZENGINE_CONFIG_HPP
