//
// Created by fotyev on 2016-11-06.
//

#include "components/config.hpp"
#include "main.hpp"
#include "components/modules.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
//#include <boost/archive/xml_iarchive.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>
#include <iostream>

#include "util/hash.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"


#include <boost/lexical_cast.hpp>



config_c::config_c()
{
    //load_from_file(configfile);
}

config_c::~config_c()
{
    //if(!configfile.empty())
        //save_to_file(configfile);
}

static bool ends_with(string_view src, string_view sub)
{
    return src.size() >= sub.size() && src.substr(src.size() - sub.size()) == sub;
}

void config_c::load_from_file(string_view filename) noexcept
{
    if(ends_with(filename, ".xml"))
        boost::property_tree::read_xml(filename.to_string(), tree);
    else if(ends_with(filename, ".json"))
        boost::property_tree::read_json(filename.to_string(), tree);
    else
        boost::property_tree::read_ini(filename.to_string(), tree);

}
void config_c::save_to_file(string_view filename) noexcept
{
    if(ends_with(filename, ".xml"))
        boost::property_tree::write_xml(filename.to_string(), tree);
    else if(ends_with(filename, ".json"))
        boost::property_tree::write_json(filename.to_string(), tree);
    else
        boost::property_tree::write_ini(filename.to_string(), tree);

}


bool config_c::set(const std::string& varname, const std::string& value)
{
    tree.put(varname, value);
    return false;
}

std::string config_c::get(const std::string& varname)
{
    try
    {
        return tree.get<std::string>(varname);
    }
    catch(boost::property_tree::ptree_error)
    {}
    return std::string{};
}


