#if 0

#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_MAX_CHANNELS 2
#define STB_VORBIS_HEADER_ONLY
#include "stb/stb_vorbis.c"

#define CUTE_SOUND_IMPLEMENTATION
#include "cute_headers/cute_sound.h"


#include "audio.hpp"
#include "util/log.hpp"

#include <stdexcept>


sound::sound(string_view filename)
{
    LOGGER(debug, "loading sound", filename);
    if(filename.find(".ogg") == filename.size() - 4)
    {
        ldh = std::make_unique<cs_loaded_sound_t>(cs_load_ogg(std::string{filename}.c_str()));
    }
    else
    {
        // wav
        ldh = std::make_unique<cs_loaded_sound_t>(cs_load_wav(std::string{filename}.c_str()));
    }
    if(ldh->channels[0] == nullptr)
        throw std::runtime_error{"cant load " + std::string{filename} + ": " + cs_error_reason};
    defh = cs_make_def(ldh.get());
}

sound::~sound()
{
    if(ldh)
    {
        LOGGER(debug, "unloading sound");
        cs_free_sound(ldh.get());
    }
}


playing_sound::playing_sound(cs_playing_sound_t * h) : h(h)
{
    if(!h)
    {
        LOGGER(error, "cant play sound, queue too small");
        static cs_playing_sound_t fake = cs_make_playing_sound(nullptr); // make fake sound
        this->h = &fake;
    }
}

mixer::mixer(void * hwnd, unsigned int play_frequency_in_Hz, int latency_factor_in_Hz, int num_buffered_seconds,
             int playing_pool_count)
{
    ctx = cs_make_context(hwnd, play_frequency_in_Hz, latency_factor_in_Hz, num_buffered_seconds, playing_pool_count);
    if(!ctx)
        throw std::runtime_error{"cant create mixer! " + std::string{cs_error_reason}};
}

#endif
