//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_COLLECTIONS_HPP
#define ZENGINE_COLLECTIONS_HPP

#include "util/lru_cache.hpp"
#include "util/geometry.hpp"
#include "util/hash.hpp"

//#include <SDL2pp/Texture.hh>
//#include <SDL2pp/Surface.hh>
#include <SDL2pp/Font.hh>

#include "opengl/texture.hpp"

#include <string>

#include "util/sdl_workaround.hpp"


//=- register_component(class=>'textures_c', name=>'textures', priority=>50);
class textures_c
{

    //using key_type = const char *;
    using key_type = uint64_t;
    using texture_type = texture;
    lru_cache_t<key_type, texture_type> cache;

    std::size_t vram_used = 0;

    // 64mb
    std::size_t vram_max;

public:
    textures_c();

    void set_max_VRAM_usage(std::size_t vram_max)
    {
        textures_c::vram_max = vram_max;
        clean();
    }

    size_t get_VRAM_used() const
    {
        return vram_used;
    }

    void clear() { return cache.clear(); }

public:

    static texture_type load_file(string_view filename);

    static std::size_t get_texture_size(const texture_type& t);


    // get texture from file
    texture_type& get_from_file(string_view filename)
    {
        //static_assert(fnv1a::hash(filename) != 0);
        return get(fnv1a::hash(filename), std::bind(load_file, filename));
    }

    template <typename Callable>
    texture_type& get(const key_type& key, const Callable& callback)
    {
        if(cache.count(key))
            return cache.get(key);
        // callback will create new texture
        return insert(key, callback());
    }

    texture_type& get(const key_type& key)
    {
        return cache.get(key);
    }


    texture_type& insert(const key_type& key, texture_type texture)
    {
        vram_used += get_texture_size(texture);
        clean();
        return cache.insert(key, std::move(texture))->second;
    }

    bool remove(const key_type& key)
    {
        auto it = cache.find(key);
        if(it == cache.end())
            return false;
        cache.erase(std::move(it));
        return true;
    }

    // from surface using default renderer
    texture_type& insert(const key_type& key, SDL2pp::Surface&& surface);


    void clean()
    {
        while(vram_used > vram_max && !cache.empty())
        {
            vram_used -= get_texture_size(cache.lru().second);
            cache.lru_remove();
        }
    }
};

//=- register_component(class=>'fonts_c', name=>'fonts', priority=>60);
class fonts_c
{
public:
    SDL2pp::Font def{"resources/DejaVuSerifCondensed.ttf", 24};

    // wrap text into maxsize rectangle
    static SDL2pp::Surface render_multiline(SDL2pp::Font& font,
                                            const std::string& str,
                                            Point maxsize,
                                            SDL_Color fg,
                                            int max_words = -1,
                                            const char * delimiters = "\n\t -");

    auto lazy_render(const std::string& str, SDL_Color color = SDL_Color{255,255,255,255})
    {
        return [&str, color{std::move(color)}, this]() {
            return this->def.RenderText_Blended(str, std::move(color)).Convert(SDL_PIXELFORMAT_RGBA32);
        };

    }
};



#endif //ZENGINE_COLLECTIONS_HPP
