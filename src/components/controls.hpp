//
// Created by fotyev on 2017-03-02.
//

#ifndef ZENGINE_CONTROLS_HPP
#define ZENGINE_CONTROLS_HPP

#include <SDL_keyboard.h>

#include <unordered_map>
#include <functional>

#include "main.hpp"
#include "components/window.hpp"


/* < register_component(class=>'controls_c', name=>'controls', priority=>75,
     scriptexport=>[qw(set_key_handler set_mouse_handler unset_key_handler unset_mouse_handler)]); >*/

class controls_c
{
public:
    using kbcallback_t = std::function<void (int)>; // state = 0 - released, 1 - pressed, 2 - pressed(repeat)
    using mousecallback_t = std::function<void (int, float, float)>; // state = 0 - released, 1 - pressed; x,y in [-1,1]

private:
    template <typename K, typename V>
    using cb_map = std::unordered_map<K, V>;

    cb_map<SDL_Scancode, kbcallback_t> kbmap;
    cb_map<uint8_t, mousecallback_t> mousemap;
    // ...

    static SDL_Scancode get_scancode(const std::string& key)
    {
        auto code = SDL_GetScancodeFromName(key.c_str());
        if(code == SDL_SCANCODE_UNKNOWN)
            throw std::runtime_error("Unknown key: " + key);
        return code;
    }

public:
    void set_key_handler(const std::string& key, const kbcallback_t& f)
    {
        kbmap[get_scancode(key)] = (f);
    }
    void unset_key_handler(const std::string& key)
    {
        kbmap.erase(get_scancode(key));
    }

    // set callback for mouse button press (1 - left, 2 - middle, 3 - right, ...) or mouse movement(button=0)
    void set_mouse_handler(int button, const mousecallback_t& f)
    {
        mousemap[static_cast<uint8_t>(button)] = (f);
    }
    void unset_mouse_handler(int button)
    {
        mousemap.erase(static_cast<uint8_t>(button));
    }

    /*
    bool on_event(const SDL_Event& ev)
    {
        switch(ev.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if(kbmap.count(ev.key.keysym.scancode))
                {
                    kbmap[ev.key.keysym.scancode]((ev.key.state == SDL_PRESSED) ? (ev.key.repeat ? 2 : 1) : 0);
                    return false;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if(mousemap.count(ev.button.button))
                {
                    //auto pos = g_app->window->to_gl_coords({ev.button.x, ev.button.y});
                    //mousemap[ev.button.button]((ev.button.state == SDL_PRESSED) ? 1 : 0, qvm::X(pos), qvm::Y(pos));
                    return false;
                }
                break;
            case SDL_MOUSEMOTION:
                if(mousemap.count(0))
                {
                    //auto pos = g_app->window->to_gl_coords({ev.motion.x, ev.motion.y});
                    //mousemap[0](0, qvm::X(pos), qvm::Y(pos));
                    return false;
                }
                break;


        }
        return true;
    }
*/
};

#endif //ZENGINE_CONTROLS_HPP
