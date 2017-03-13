//
// Created by fotyev on 2016-10-19.
//
#include "main.hpp"
#include "modules.hpp"
#include "components/script.hpp"

modules_c::modules_c()
{
}

void modules_c::init()
{
    //=- register_callback('void ()', 'on_init');
    //g_app->script->eval("on_init();");
    g_app->script->on_init();

}

bool modules_c::on_event(const SDL_Event& ev)
{
    for(const auto& module : modules)
    {
        if(!module.second->on_event(ev))
            return false;
    }
    return true;
}

void modules_c::draw()
{
    for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        it->second->draw();
    }
}

basic_module * modules_c::get(const modules_c::key_type& key)
{
    auto it = modules.find(key);
    if(it == modules.end())
        return nullptr;
    return it->second.get();
}

modules_c::~modules_c()
{
    // TODO: destroy them in reverse(?) order
    for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        it->second.reset();
    }
}

void modules_c::unload_module(basic_module * ptr)
{
    for(auto it = modules.begin(); it != modules.end(); ++it)
    {
        if(it->second.get() == ptr)
        {
            modules.erase(it);
            return;
        }
    }
}
