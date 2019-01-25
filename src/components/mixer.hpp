//
// Created by fotyev on 2018-07-14.
//

#ifndef ZENGINE_MIXER_HPP
#define ZENGINE_MIXER_HPP

#include "util/audio.hpp"

#include "main.hpp"
#include "components/window.hpp"
#include "util/semiconfig.hpp"

// depends on window
//= register_component(class=>'mixer_c', name=>'mixer', priority=>70);

class mixer_c : public mixer
{
public:
    mixer_c() : mixer(g_app->window->get_hwnd(),
                      SCFG(audio.frequency_hz, 44100),
                      SCFG(audio.latency_hz, 8),
                      SCFG(audio.buffer_s, 1.f),
                      SCFG(audio.channels, 16)
                      )
    {
        if(SCFG(audio.enabled, true))
            this->spawn_mix_thread(SCFG(audio.sleep_ms, 5));
    }
};

#endif //ZENGINE_MIXER_HPP
