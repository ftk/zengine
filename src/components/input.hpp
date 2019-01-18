//
// Created by fotyev on 2018-11-23.
//

#ifndef ZENGINE_INPUT_HPP
#define ZENGINE_INPUT_HPP

#include <cstdint>
#include <unordered_map>
#include <string>
#include "util/signals.hpp"

//=- register_component(class=>'input_map_c', name=>'input', priority=>30, scriptexport=>['']);
class input_map_c
{
public:
    using key_t = uint32_t;
    using bind_t = const char *;
    using button_sig = sig::signal<void (bool)>;
private:
    std::unordered_multimap<key_t, bind_t> key_map;
    std::unordered_map<bind_t, button_sig> button_map;

    sig::scoped_connection _key_handler, _jsb_handler, _mouse_handler;

public:
    input_map_c();

    bool register_key(const char * keyname, bind_t bind);

    static key_t to_key(const char *);

    button_sig& button(bind_t name, const char * default_key = nullptr);
    //button_sig& button_s(const std::string& name, const std::string& default_key) { return button(name.c_str(), default_key.empty() ? nullptr : default_key.c_str());}
private:
    void on_key(key_t key, bool state) const;
};

#endif //ZENGINE_INPUT_HPP
