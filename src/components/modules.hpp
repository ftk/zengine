//
// Created by fotyev on 2016-10-16.
//

#ifndef ZENGINE_MODULES_HPP
#define ZENGINE_MODULES_HPP

#include <boost/container/flat_map.hpp>
#include <memory>
#include "modules/basic_module.hpp"

#include "util/assert.hpp"

//=- register_component(class=>'modules_c', name=>'modules', priority=>0, scriptexport=>[qw(get loaded unload)]);
// register_module(class=>'modules_c', priority=>0, scriptexport=>[qw(get loaded unload)]);
class modules_c //: public basic_module
{
public:
    typedef int key_type;
private:
    boost::container::flat_map<key_type, std::unique_ptr<basic_module> > modules;

    //std::map<key_type, std::unique_ptr<basic_module> > modules;
public:
    modules_c();
    ~modules_c();

    void init();
    void clear() { modules.clear(); }

    bool on_event(const SDL_Event& ev);

    void draw();

    basic_module * get(const key_type& key);

    bool loaded(const key_type& key) const
    {
        return modules.count(key);
    }

    template <class T, typename... Args>
    T * load(const key_type& key, const Args&... args) // perfect forwarding doesn't work with chaiscript:((
    {
        T * ptr = new T(args...);
        try
        {
            assume(!modules.count(key));
            modules.insert(std::make_pair(key, ptr));
            return ptr;
        }
        catch(...)
        {
            delete ptr;
            throw;
        }
    }

    void unload(const key_type& key)
    {
        modules.erase(key);
    }

    void unload_module(basic_module * ptr);
};



#endif //ZENGINE_MODULES_HPP
