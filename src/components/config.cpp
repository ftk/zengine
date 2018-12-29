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

#ifdef WIN32 // for chdir
#include <direct.h>
#else
#include <unistd.h>
#endif


config_c::config_c()
{
}

config_c::config_c(string_view default_file, int argc, const char * const * argv) : tree{}
{
    load_from_file(default_file);

    int argn;
    for(argn = 1; argn < argc; argn++)
    {
        auto arg = string_view{argv[argn]};
        std::size_t eq_pos = arg.find('=');
        if(eq_pos == string_view::npos)
            break;

        auto key = arg.substr(0, eq_pos);
        auto value = arg.substr(eq_pos + 1);

        if(key == "file")
        {
            if(!value.empty())
            {
                g_app->config->load_from_file(value);
            }
        } else
            g_app->config->set(std::string{key}, std::string{value});
    }

    if(auto dir = get_optional<std::string>("app.dir"))
        chdir(dir->c_str());
#if !defined(LOG_DISABLE) && !defined(LOG_HEADER_ONLY)
    if(auto filename = get_optional<std::string>("app.log.file"))
    {
        static std::ofstream logfile(*filename);
        loggers().push_back({logfile, log_level::all, log_detail::TIME});
    }
#endif
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
    catch(const file_parser_error& e)
    {
        LOGGER(error, "config:", e.what());
    }
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
    catch(const file_parser_error& e)
    {
        LOGGER(error, "config:", e.what());
    }
    return false;
}


bool config_c::set_param(const std::string& varname, const std::string& value)
{
    tree.put(varname, value);
    return false;
}

std::string config_c::get_param(const std::string& varname)
{
    using namespace boost::property_tree;
    try
    {
        return tree.get<std::string>(varname);
    }
    catch(const file_parser_error& e)
    {
        LOGGER(error, "config:", e.what());
    }
    return std::string{};
}



