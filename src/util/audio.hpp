//
// Created by fotyev on 2018-07-14.
//

#ifndef ZENGINE_AUDIO_HPP
#define ZENGINE_AUDIO_HPP

#define STB_VORBIS_INCLUDE_STB_VORBIS_H
#include "cute_headers/cute_sound.h"

#include "util/movable.hpp"
#include "util/assert.hpp"

#include <string_view>
#include <memory>

class sound
{
private:
    cs_play_sound_def_t defh;
    std::unique_ptr<cs_loaded_sound_t> ldh;
    NONCOPYABLE_BUT_SWAPPABLE(sound,(defh)(ldh))
    friend class mixer;
    friend class simple_sound;

    sound() = default;
public:
    explicit sound(std::string_view filename);

    ~sound();

    int size() const { return cs_sound_size(const_cast<cs_loaded_sound_t*>(ldh.get())); }
};

class playing_sound
{
    cs_playing_sound_t * h = nullptr;
    NONCOPYABLE_BUT_SWAPPABLE(playing_sound,(h))
    friend class mixer;
    friend class simple_sound;

    playing_sound() = default;
    explicit playing_sound(cs_playing_sound_t * h);
public:
#define CWRAPPER(method,cname) auto method (auto... args) { return cname (h, args...); }
    CWRAPPER(active,cs_is_active)
    CWRAPPER(stop,cs_stop_sound)
    CWRAPPER(loop,cs_loop_sound)
    CWRAPPER(pause,cs_pause_sound)
    CWRAPPER(pan,cs_set_pan)
    CWRAPPER(volume,cs_set_volume)
    CWRAPPER(pitch,cs_set_pitch)
#undef CWRAPPER

};

class mixer
{
    cs_context_t * ctx = nullptr;
    NONCOPYABLE_BUT_SWAPPABLE(mixer,(ctx))
public:
    mixer(void * hwnd, unsigned play_frequency_in_Hz = 44100, int latency_factor_in_Hz = 8, int num_buffered_seconds = 1, int playing_pool_count = 32);
    ~mixer() { if(ctx) cs_shutdown_context(ctx); }

    // either call spawn_mix_thread once or call mix every tick
    void spawn_mix_thread(int delay = 5)
    {
        cs_thread_sleep_delay(ctx, delay);
        cs_spawn_mix_thread(ctx);
    }

    playing_sound play(const sound& snd)
    {
        return playing_sound{cs_play_sound(ctx,snd.defh)};
    }

#define CWRAPPER(method,cname) auto method () { return cname (ctx); }
    CWRAPPER(stop_sounds,cs_stop_all_sounds)
    CWRAPPER(mix,cs_mix)
#undef CWRAPPER


};

class simple_sound
{
    sound snd;
    playing_sound pl;
    NONCOPYABLE_BUT_SWAPPABLE(simple_sound,(snd)(pl))
public:
    simple_sound() = delete;
    simple_sound(mixer& mx, std::string_view filename) : snd(filename), pl{mx.play(snd)}
    {
    }

    simple_sound(mixer& mx, sound&& snd) : snd(std::move(snd)), pl{mx.play(snd)}
    {
    }

    ~simple_sound()
    {
        pl.stop();
    }
};


#include "util/resource_traits.hpp"

template <>
struct resource_traits<sound>
{
    static unsigned int get_size(const sound& rsc)
    {
        return static_cast<unsigned>(rsc.size());
    }
    static sound from_id(std::string_view id) { return sound{id}; }
};



#endif //ZENGINE_AUDIO_HPP
