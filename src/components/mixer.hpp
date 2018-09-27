//
// Created by fotyev on 2018-07-14.
//

#ifndef ZENGINE_MIXER_HPP
#define ZENGINE_MIXER_HPP

#include "util/audio.hpp"

#include "main.hpp"
#include "components/window.hpp"
#include "components/config.hpp"

// depends on window
//=- register_component(class=>'mixer_c', name=>'mixer', priority=>70);

class mixer_c : public mixer
{
public:
    mixer_c() : mixer(g_app->window->get_hwnd(),
                      g_app->config->get("audio.frequency_hz", 44100),
                      g_app->config->get("audio.latency_hz", 8),
                      g_app->config->get("audio.buffer_s", 1.f),
                      g_app->config->get("audio.channels", 16)
                      )
    {
        if(g_app->config->get("audio.enabled", true))
            this->spawn_mix_thread(g_app->config->get("audio.sleep_ms", 5));
    }
};

#endif //ZENGINE_MIXER_HPP
