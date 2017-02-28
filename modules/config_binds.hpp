//
// Created by fotyev on 2016-11-28.
//

#ifndef ZENGINE_CONFIG_BINDS_HPP
#define ZENGINE_CONFIG_BINDS_HPP

#include "basic_module.hpp"
#include "components/config.hpp"

#include "main.hpp"

class config_binds : public basic_module
{
    bool on_event(const SDL_Event& ev) override
    {
        return g_app->config->on_event(ev);
    }
};


#endif //ZENGINE_CONFIG_BINDS_HPP
