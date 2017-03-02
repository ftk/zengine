//
// Created by fotyev on 2017-03-02.
//

#ifndef ZENGINE_CONTROLS_HPP
#define ZENGINE_CONTROLS_HPP

#include "modules/basic_module.hpp"

#include <SDL_keyboard.h>

#include <unordered_map>
#include <functional>

#include "main.hpp"
#include "components/window.hpp"


/*< register_module(class=>'controls', name=>'controls',
     scriptexport=>[qw(set_key_handler set_mouse_handler)]); >*/

class controls : public basic_module
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

public:
    void set_key_handler(const std::string& key, const kbcallback_t& f)
    {
        kbmap[SDL_GetScancodeFromName(key.c_str())] = (f);
    }

    void set_mouse_handler(int button, const mousecallback_t& f)
    {
        mousemap[static_cast<uint8_t>(button)] = (f);
    }

    bool on_event(const SDL_Event& ev) override
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
                    float x = float(2 * ev.button.x) / g_app->window->get_size().x - 1;
                    float y = float(-2 * ev.button.y) / g_app->window->get_size().y + 1;
                    mousemap[ev.button.button]((ev.button.state == SDL_PRESSED) ? 1 : 0, x, y);
                    return false;
                }
                break;

        }
        return true;
    }

};

#endif //ZENGINE_CONTROLS_HPP
