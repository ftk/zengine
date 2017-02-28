//
// Created by fotyev on 2017-01-25.
//

#ifndef ZENGINE_OPTIONBOX_HPP
#define ZENGINE_OPTIONBOX_HPP

#include <main.hpp>
#include "basic_module.hpp"

#include "opengl/opengl.hpp"

#include "components/collections.hpp"
#include "components/window.hpp"

#include <vector>

#include "components/script.hpp"


/*< register_module(class=>'optionbox', name=>'optionbox',
    scriptexport=>['add', 'clear', 'update', 'set_offset'],
    ctors=>['','float,float']
    ); >*/

class optionbox : public basic_module
{
    NONCOPYABLE(optionbox);
public:
    optionbox() = default;
    optionbox(float x, float y) { offset = {x, y}; }
private:
    std::vector<std::string> options = {"option 1", "option 2", "option 3"};
    //uint64_t hash;
    int selected = 0;
    qvm::vec2 offset {0.0, 0.0};


    float opt_height = 0.025f * 2; // each opt is 2.5% of screen height
public:

    void add(std::string opt) { options.push_back(std::move(opt)); }
    void clear() { options.clear(); }
    void set_offset(float x, float y) { offset = {x, y}; }

    void update()
    {
        int margin_x = 20, line_y = g_app->fonts->def.GetLineSkip();
        Point maxsize { 300, (int)options.size() * line_y};
        SDL2pp::Surface rect = create_optimized_surface(maxsize);

        // TODO:endianness
        rect.FillRect(SDL2pp::NullOpt, 0x60000000); // dark box


        for(std::size_t i = 0; i < options.size(); i++)
        {
            auto surf = g_app->fonts->def.RenderUTF8_Blended(options[i], SDL_Color{255, 255, 255, 255});
            surf.Blit(SDL2pp::NullOpt, rect, Rect{margin_x, line_y * (int)i, maxsize.x, line_y});
        }
        //hash = fnv1a::foldr(options.size(), options[0]);
        g_app->textures->insert("textbox"_fnv, texture{std::move(rect)});
    }

    void draw()
    {
        if(options.empty())
            return;
        gl::Enable(GL_BLEND);
        gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl::Disable(GL_DEPTH_TEST);
        g_app->window->render.set4dpos();

        using namespace qvm;
        texture& box = g_app->textures->get("textbox"_fnv);
        g_app->window->render.copy(offset + vec2{0.0f, -opt_height * options.size()}, offset + vec2{0.45f, 0.0f}, nullopt, box);
        if(SDL_GetTicks() % 300 < 150) //blink
        {
            texture& pointer = g_app->textures->get("pointer"_fnv,
                                                    [](){ return texture::from_surface(
                                                            g_app->fonts->def.RenderUTF8_Blended(
                                                                    //►▶
                                                                    "►", SDL_Color{255, 255, 255, 255}));
                                                    });
            g_app->window->render.copy(offset + vec2{0.f, -opt_height * (selected + 1)},
                                       offset + vec2{0.01f, -opt_height * (selected)}, nullopt, pointer);
        }
    }

    bool on_event(const SDL_Event& ev) override
    {
        if(options.empty())
            return true;
        switch(ev.type)
        {
            case SDL_KEYDOWN:
            {
                int dir = 1;
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_KP_8:
                    case SDL_SCANCODE_UP:
                        dir = -1;
                    case SDL_SCANCODE_KP_2:
                    case SDL_SCANCODE_DOWN:
                        selected += dir;
                        if(selected >= (int)options.size())
                            selected = (ev.key.repeat) ? options.size() - 1 : 0;
                        else if(selected <= -1)
                            selected = (!ev.key.repeat) ? options.size() - 1 : 0;

                        return false;
                    case SDL_SCANCODE_KP_5:
                    case SDL_SCANCODE_KP_ENTER:
                    case SDL_SCANCODE_RETURN:
                        //=- register_callback('void (int)', 'on_option_selected');
                        g_app->script->on_option_selected(selected);
                        return false;
                    default:
                        return true;

                }
            }
            default:
                return true;
        }
        return true;
    }


};

#endif //ZENGINE_OPTIONBOX_HPP
