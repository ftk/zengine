//
// Created by fotyev on 2016-10-15.
//

#include "main.hpp"
#include "collections.hpp"
#include "window.hpp"

#include "config.hpp"

#include <SDL2pp/Surface.hh>
/*
auto textures_c::load_file(string_view filename) -> texture_type
{
    return texture_type{g_app->window->render(), filename.to_string()};
}

textures_c::texture_type& textures_c::insert(const textures_c::key_type & key, const SDL2pp::Surface& surface)
{
    return insert(key, texture_type{g_app->window->render(), surface});
}


std::size_t textures_c::get_texture_size(const texture_type& t)
{
     return SDL_BYTESPERPIXEL(t.GetFormat()) * t.GetHeight() * t.GetWidth();
}
*/

auto textures_c::load_file(string_view filename) -> texture_type
{
    return texture_type::from_file(filename);
}

textures_c::texture_type& textures_c::insert(const textures_c::key_type & key, SDL2pp::Surface&& surface)
{
    return insert(key, texture_type{std::move(surface)});
}


std::size_t textures_c::get_texture_size(const texture_type& t)
{
    return 4 * t.get_size().x * t.get_size().y;
}

textures_c::textures_c() : vram_max(g_app->config->tree.get("texture.cache", 64 * 1024 * 1024))
{

}


SDL2pp::Surface
fonts_c::render_multiline(SDL2pp::Font& font, const std::string& str, Point maxsize, SDL_Color fg,
                          int max_words, const char * delimiters)
{

    SDL2pp::Surface rect = create_optimized_surface(maxsize);


    std::string line;
    Point curpos{0,0};

    std::size_t pos = 0, prev_pos;
    --pos;
    int words = -1;
    do
    {
        words++;
        prev_pos = pos;
        pos = str.find_first_of(delimiters, prev_pos + 1);

        bool newline = false;
        if(!line.empty() && str[prev_pos] == '\n')
        {
            line.pop_back();
            //prev_pos++;
            newline = true;
        }
        auto substr = str.substr(prev_pos + 1, (pos == std::string::npos) ? pos : (pos - prev_pos));

        if(!substr.empty())
            curpos.x += font.GetSizeUTF8(substr).x;
        if(curpos.x > maxsize.x || newline || words == max_words)
        {

            // split
            if(!line.empty())
            {
                //if(!font.GetSizeUTF8(line).x)
                    //throw std::runtime_error("test");
                auto surf = font.RenderUTF8_Blended(line, fg);
                surf.Blit(SDL2pp::NullOpt, rect, Rect(0, curpos.y, maxsize.x, font.GetLineSkip()));
            }

            curpos.y += font.GetLineSkip();
            if(curpos.y >= maxsize.y || words == max_words)
                break;
            line = substr;

            curpos.x = line.empty() ? 0 : font.GetSizeUTF8(line).x;

        } else
            line += substr;
    }
    while(pos != std::string::npos);

    if(curpos.y < maxsize.y && words != max_words && !line.empty())
    {
        auto surf = font.RenderUTF8_Blended(line, fg);
        surf.Blit(SDL2pp::NullOpt, rect, Rect(0, curpos.y, maxsize.x, font.GetLineSkip()));
    }

    return rect;
}
