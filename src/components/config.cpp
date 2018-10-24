//
// Created by fotyev on 2016-11-06.
//

#include "components/config.hpp"
#include "main.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>
#include <iostream>

#include "util/hash.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"


config_c::config_c()
{
}

config_c::~config_c()
{
    if(auto file = get_optional<std::string>("config.file"))
        save_to_file(*file);
}

static bool ends_with(string_view src, string_view sub)
{
    return src.size() >= sub.size() && src.substr(src.size() - sub.size()) == sub;
}

bool config_c::load_from_file(string_view filename) noexcept
{
    using namespace boost::property_tree;
    try
    {
        if(ends_with(filename, ".xml"))
            read_xml(std::string{filename}, tree, xml_parser::trim_whitespace);
        else if(ends_with(filename, ".json"))
            read_json(std::string{filename}, tree);
        else
            read_ini(std::string{filename}, tree);
        return true;
    }
    catch(file_parser_error)
    {}
    return false;
}
bool config_c::save_to_file(string_view filename) noexcept
{
    using namespace boost::property_tree;
    try
    {
        if(ends_with(filename, ".xml"))
            write_xml(std::string{filename}, tree, std::locale(), {'\t', 1});
        else if(ends_with(filename, ".json"))
            write_json(std::string{filename}, tree);
        else
            write_ini(std::string{filename}, tree);
        return true;
    }
    catch(file_parser_error)
    {}
    return false;
}


bool config_c::set_param(const std::string& varname, const std::string& value)
{
    tree.put(varname, value);
    return false;
}

std::string config_c::get_param(const std::string& varname)
{
    try
    {
        return tree.get<std::string>(varname);
    }
    catch(boost::property_tree::ptree_error)
    {}
    return std::string{};
}



