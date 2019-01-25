//
// Created by fotyev on 2016-11-06.
//

#ifndef ZENGINE_CONFIG_HPP
#define ZENGINE_CONFIG_HPP


#include "util/semiconfig.hpp"
#include "util/string.hpp"

#include <algorithm>
#include <vector>
#include <fstream>

//=- register_component(class=>'config_c', name=>'config', priority=>0, scriptexport=>[qw(get set)]);
class config_c
{
public:

    // name=value
    void parse_and_set(string_view line) noexcept
    {
        while(!line.empty() && line[0] == '-') // remove leading dashes
            line.remove_prefix(1);
        auto spl = string_split<2>(line, '=');
        if(!spl[1].empty())
            static_config::set(std::string{spl[0]}, std::string{spl[1]});
    }

	bool load_from_file(string_view filename) noexcept
    {
        std::ifstream cfg(std::string{filename});
        if(!cfg)
            return false;

        std::string line;
        while(std::getline(cfg, line))
        {
            if(line.empty() || line[0] == '#' || line[0] == ';')
                continue;
            while(line.back() == '\r') line.pop_back();
            parse_and_set(line);
        }
        return true;
    }

    bool save_to_file(string_view filename) noexcept
    {
        std::ofstream cfg(std::string{filename});
        if(!cfg)
            return false;
        // sort first
        std::vector<std::pair<std::string, std::string>> nvps;
        nvps.reserve(static_config::size());
        static_config::for_each([&nvps](std::string name, std::string val) { nvps.emplace_back(std::move(name), std::move(val)); });
        std::sort(nvps.begin(), nvps.end());
        for(const auto& nvp : nvps)
            cfg << nvp.first << '=' << nvp.second << std::endl;
        return true;
    }

    config_c() = default;

    config_c(string_view default_file, int argc, const char * const * argv) noexcept
    {
        load_from_file(default_file);
        for(int i = 1; i < argc; i++)
        {
            parse_and_set(argv[i]);
        }
        auto& file = static_config::at_optional<std::string>(ID("config.file"));
        if(file)
            load_from_file(*file);
        LOGGER(info, SCFG(test1, "test1"));
        LOGGER(info, SCFG(test1, "test2"));
        //else
            //file.emplace(default_file);
    }

    ~config_c()
    {
        if(SCFG(config.file.overwrite, true))
            save_to_file("ccc"/*SCFG(config.file, "ccc")*/);
    }

    auto get(const std::string& varname)
    {
        return static_config::get(varname);
    }
    bool set(const std::string& varname, std::string value)
    {
        return static_config::set(varname, std::move(value));
    }

};

#endif //ZENGINE_CONFIG_HPP
