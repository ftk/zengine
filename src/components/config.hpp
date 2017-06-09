//
// Created by fotyev on 2016-11-06.
//

#ifndef ZENGINE_CONFIG_HPP
#define ZENGINE_CONFIG_HPP

#include "modules/basic_module.hpp"
#include <string>
//#include <set>
#include <type_traits>

#include <boost/property_tree/ptree.hpp>

//#include <vector>
#include "util/hash.hpp"
#include "util/assert.hpp"

//#include <boost/serialization/nvp.hpp>

#include <boost/noncopyable.hpp>


//=- register_component(class=>'config_c', name=>'config', priority=>0, scriptexport=>[qw(set)]);
class config_c : boost::noncopyable
{

	boost::property_tree::ptree tree;
public:

	std::string configfile = "config.xml";
public:


	bool load_from_file(string_view filename) noexcept;
    bool save_to_file(string_view filename) noexcept;
    config_c();
    ~config_c();

    template <typename Type>
    auto get(const std::string& varname, Type def)
    {
        using TypeFixed = std::conditional_t<std::is_same<Type, const char *>::value, std::string, Type>;
        if(auto opt = tree.get_optional<TypeFixed>(varname))
        {
            return *opt;
        }
        tree.put(varname, def);
        return static_cast<TypeFixed>(def);
    }
    template <typename Type>
    auto get_optional(const std::string& varname) { return tree.get_optional<Type>(varname); }

	bool set(const std::string& varname, const std::string& value);
	std::string get(const std::string& varname);


    /*< #serialize dispatch('config_save'); %*//*>*/
};

#endif //ZENGINE_CONFIG_HPP
