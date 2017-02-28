//
// Created by fotyev on 2016-10-16.
//

#ifndef ZENGINE_MIXER_HPP
#define ZENGINE_MIXER_HPP

#include <SDL2pp/Mixer.hh>

//= register_component(class=>'mixer_c', name=>'mixer', priority=>75);
class mixer_c
{

public:
    SDL2pp::Mixer mixer{MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096};

};

#endif //ZENGINE_MIXER_HPP
