//
// Created by fotyev on 2018-11-23.
//

#include "input.hpp"

#include <GLFW/glfw3.h>
#include <boost/lexical_cast.hpp>
#include <cstring>

#include "components/window.hpp"
#include "util/semiconfig.hpp"
#include "components/joystick.hpp"

#include "main.hpp"

#include "util/log.hpp"
#include "util/assert.hpp"
#include "util/string.hpp"



//  sed  -n 's/^#define \(GLFW_KEY_\([A-Z0-9_]*\)\).*$/{"\2", \1},/p' glfw3.h
static const std::unordered_map<std::string, input_map_c::key_t> keymap = {
        {"SPACE", GLFW_KEY_SPACE},
        {"APOSTROPHE", GLFW_KEY_APOSTROPHE},
        {"COMMA", GLFW_KEY_COMMA},
        {"MINUS", GLFW_KEY_MINUS},
        {"PERIOD", GLFW_KEY_PERIOD},
        {"SLASH", GLFW_KEY_SLASH},
        {"0", GLFW_KEY_0},
        {"1", GLFW_KEY_1},
        {"2", GLFW_KEY_2},
        {"3", GLFW_KEY_3},
        {"4", GLFW_KEY_4},
        {"5", GLFW_KEY_5},
        {"6", GLFW_KEY_6},
        {"7", GLFW_KEY_7},
        {"8", GLFW_KEY_8},
        {"9", GLFW_KEY_9},
        {"SEMICOLON", GLFW_KEY_SEMICOLON},
        {"EQUAL", GLFW_KEY_EQUAL},
        {"A", GLFW_KEY_A},
        {"B", GLFW_KEY_B},
        {"C", GLFW_KEY_C},
        {"D", GLFW_KEY_D},
        {"E", GLFW_KEY_E},
        {"F", GLFW_KEY_F},
        {"G", GLFW_KEY_G},
        {"H", GLFW_KEY_H},
        {"I", GLFW_KEY_I},
        {"J", GLFW_KEY_J},
        {"K", GLFW_KEY_K},
        {"L", GLFW_KEY_L},
        {"M", GLFW_KEY_M},
        {"N", GLFW_KEY_N},
        {"O", GLFW_KEY_O},
        {"P", GLFW_KEY_P},
        {"Q", GLFW_KEY_Q},
        {"R", GLFW_KEY_R},
        {"S", GLFW_KEY_S},
        {"T", GLFW_KEY_T},
        {"U", GLFW_KEY_U},
        {"V", GLFW_KEY_V},
        {"W", GLFW_KEY_W},
        {"X", GLFW_KEY_X},
        {"Y", GLFW_KEY_Y},
        {"Z", GLFW_KEY_Z},
        {"LEFT_BRACKET", GLFW_KEY_LEFT_BRACKET},
        {"BACKSLASH", GLFW_KEY_BACKSLASH},
        {"RIGHT_BRACKET", GLFW_KEY_RIGHT_BRACKET},
        {"GRAVE_ACCENT", GLFW_KEY_GRAVE_ACCENT},
        {"WORLD_1", GLFW_KEY_WORLD_1},
        {"WORLD_2", GLFW_KEY_WORLD_2},
        {"ESCAPE", GLFW_KEY_ESCAPE},
        {"ENTER", GLFW_KEY_ENTER},
        {"TAB", GLFW_KEY_TAB},
        {"BACKSPACE", GLFW_KEY_BACKSPACE},
        {"INSERT", GLFW_KEY_INSERT},
        {"DELETE", GLFW_KEY_DELETE},
        {"RIGHT", GLFW_KEY_RIGHT},
        {"LEFT", GLFW_KEY_LEFT},
        {"DOWN", GLFW_KEY_DOWN},
        {"UP", GLFW_KEY_UP},
        {"PAGE_UP", GLFW_KEY_PAGE_UP},
        {"PAGE_DOWN", GLFW_KEY_PAGE_DOWN},
        {"HOME", GLFW_KEY_HOME},
        {"END", GLFW_KEY_END},
        {"CAPS_LOCK", GLFW_KEY_CAPS_LOCK},
        {"SCROLL_LOCK", GLFW_KEY_SCROLL_LOCK},
        {"NUM_LOCK", GLFW_KEY_NUM_LOCK},
        {"PRINT_SCREEN", GLFW_KEY_PRINT_SCREEN},
        {"PAUSE", GLFW_KEY_PAUSE},
        {"F1", GLFW_KEY_F1},
        {"F2", GLFW_KEY_F2},
        {"F3", GLFW_KEY_F3},
        {"F4", GLFW_KEY_F4},
        {"F5", GLFW_KEY_F5},
        {"F6", GLFW_KEY_F6},
        {"F7", GLFW_KEY_F7},
        {"F8", GLFW_KEY_F8},
        {"F9", GLFW_KEY_F9},
        {"F10", GLFW_KEY_F10},
        {"F11", GLFW_KEY_F11},
        {"F12", GLFW_KEY_F12},
        {"F13", GLFW_KEY_F13},
        {"F14", GLFW_KEY_F14},
        {"F15", GLFW_KEY_F15},
        {"F16", GLFW_KEY_F16},
        {"F17", GLFW_KEY_F17},
        {"F18", GLFW_KEY_F18},
        {"F19", GLFW_KEY_F19},
        {"F20", GLFW_KEY_F20},
        {"F21", GLFW_KEY_F21},
        {"F22", GLFW_KEY_F22},
        {"F23", GLFW_KEY_F23},
        {"F24", GLFW_KEY_F24},
        {"F25", GLFW_KEY_F25},
        {"KP_0", GLFW_KEY_KP_0},
        {"KP_1", GLFW_KEY_KP_1},
        {"KP_2", GLFW_KEY_KP_2},
        {"KP_3", GLFW_KEY_KP_3},
        {"KP_4", GLFW_KEY_KP_4},
        {"KP_5", GLFW_KEY_KP_5},
        {"KP_6", GLFW_KEY_KP_6},
        {"KP_7", GLFW_KEY_KP_7},
        {"KP_8", GLFW_KEY_KP_8},
        {"KP_9", GLFW_KEY_KP_9},
        {"KP_DECIMAL", GLFW_KEY_KP_DECIMAL},
        {"KP_DIVIDE", GLFW_KEY_KP_DIVIDE},
        {"KP_MULTIPLY", GLFW_KEY_KP_MULTIPLY},
        {"KP_SUBTRACT", GLFW_KEY_KP_SUBTRACT},
        {"KP_ADD", GLFW_KEY_KP_ADD},
        {"KP_ENTER", GLFW_KEY_KP_ENTER},
        {"KP_EQUAL", GLFW_KEY_KP_EQUAL},
        {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT},
        {"LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL},
        {"LEFT_ALT", GLFW_KEY_LEFT_ALT},
        {"LEFT_SUPER", GLFW_KEY_LEFT_SUPER},
        {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
        {"RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
        {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT},
        {"RIGHT_SUPER", GLFW_KEY_RIGHT_SUPER},
        {"MENU", GLFW_KEY_MENU},
};

enum : uint32_t
{
    MOUSE_BUTTON = 1u << 16,
    JOYSTICK_BUTTON = 1u << 17,
    MODIFIER_OFFSET = 17u,
    MODIFIER_MASK = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER,
};

static inline uint32_t apply_mods(uint32_t mods) {
    return (mods & MODIFIER_MASK) << MODIFIER_OFFSET;
}

input_map_c::key_t input_map_c::to_key(const char * keyname)
{
    if(keymap.count(keyname))
        return keymap.find(keyname)->second;
    if(atoi(keyname) > 0)
        return (unsigned)atoi(keyname);
    {
        auto splitted = string_split<2>(keyname, " -=");
        if(splitted[0] == "JSB")
            return JOYSTICK_BUTTON | boost::lexical_cast<unsigned>(splitted[1]);
        if(splitted[0] == "MOUSE")
            return MOUSE_BUTTON | boost::lexical_cast<unsigned>(splitted[1]);
    }
    // SHIFT+CTRL+KEY
    auto splitted = string_split_v(keyname, '+');
    if(splitted.size() > 1)
    {
        auto it = splitted.rbegin();
        auto key = to_key(std::string{*it}.c_str());
        if(key)
        {
            for(++it; it != splitted.rend(); ++it)
            {
                if(*it == "SHIFT")
                    key |= apply_mods(GLFW_MOD_SHIFT);
                else if(*it == "CTRL")
                    key |= apply_mods(GLFW_MOD_CONTROL);
                else if(*it == "ALT")
                    key |= apply_mods(GLFW_MOD_ALT);
                else if(*it == "SUPER")
                    key |= apply_mods(GLFW_MOD_SUPER);
                else
                    return 0;
            }
            return key;
        }
    }
    return 0;
}

input_map_c::button_sig& input_map_c::button(bind_t name, const char * default_key)
{
    if(default_key && !button_map.count(name)) // registering first time
    {
        auto keystr = static_config::get(std::string{"input."} + name);
        if(keystr.empty())
        {
            keystr = default_key;
            static_config::set(std::string{"input."} + name, keystr);
        }

        int i = 2;
        while(register_key(keystr.c_str(), name))
        {
            LOGGER(debug, name, "mapped to", keystr);
            if(auto next = static_config::get_optional(std::string{"input."} + name + std::to_string(i++)))
                keystr = *next;
            else
                break;
        }
    }
    return button_map[name];
}

void input_map_c::on_key(key_t key, bool state) const
{
    auto rng = key_map.equal_range(key);
    for(auto it = rng.first; it != rng.second; ++it)
    {
        assert(button_map.count(it->second));
        (button_map.find(it->second)->second)(state);
    }
}


bool input_map_c::register_key(const char * keyname, bind_t bind)
{
    auto key = to_key(keyname);
    if(!key)
    {
        LOGGER(error, "no key", keyname);
        return false;
    }
    if(key_map.count(key))
        LOGGER(warn, keyname, "is already mapped");

    key_map.emplace(key, bind);
    return true;
}

input_map_c::input_map_c() :
_key_handler{g_app->window->key.connect([this](auto k) {
    if(k.action == GLFW_RELEASE || k.action == GLFW_PRESS)
    {
        on_key(k.key, k.action == GLFW_PRESS);
        if(apply_mods(k.mods))
            on_key(k.key | apply_mods(k.mods), k.action == GLFW_PRESS);
    }
})},
_jsb_handler{g_app->joystick->button.connect([this](unsigned int button, bool state){
    on_key(JOYSTICK_BUTTON | button, state);
})},
_mouse_handler{g_app->window->mouse_click.connect([this](auto e) {
    on_key(MOUSE_BUTTON | e.button, e.action == GLFW_PRESS);
    if(apply_mods(e.mods))
        on_key(MOUSE_BUTTON | e.button | apply_mods(e.mods), e.action == GLFW_PRESS);
})}
{
}
