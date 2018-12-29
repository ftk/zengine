//
// Created by fotyev on 2016-11-06.
//

#ifndef ZENGINE_CONFIG_HPP
#define ZENGINE_CONFIG_HPP

#include <string>
#include <boost/property_tree/ptree.hpp>
#include "util/hash.hpp"


//=- register_component(class=>'config_c', name=>'config', priority=>0, scriptexport=>[qw(get_param set_param)]);
class config_c
{
	boost::property_tree::ptree tree;
public:

	bool load_from_file(string_view filename) noexcept;
    bool save_to_file(string_view filename) noexcept;
    config_c();
    config_c(string_view default_file, int argc, const char * const * argv);
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

    template <typename Type>
    void set(const std::string& varname, Type value)
    {
        tree.put(varname, std::move(value));
    }

    std::string get_param(const std::string& varname);
	bool set_param(const std::string& varname, const std::string& value);
};

#endif //ZENGINE_CONFIG_HPP
