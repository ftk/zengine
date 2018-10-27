//
// Created by fotyev on 2018-07-13.
//

#ifndef ZENGINE_RESOURCES_HPP
#define ZENGINE_RESOURCES_HPP

#include "util/resource_cache.hpp"

#include "opengl/texture.hpp"
#include "opengl/textrender.hpp"


//#include "util/audio.hpp"


/*
#include "audio.h"

template <>
struct resource_traits<Sound>
{
    static unsigned int get_size(const Sound& rsc)
    {
        return 1;
    }
    static Sound from_id(string_view id) { return LoadSound(std::string{id}.c_str()); }
};
*/
//=- register_component(class=>'resources_c', name=>'resources', priority=>50);

class resources_c
{
public:
    resource_cache<texture> textures{8 * 1024 * 1024};
    //resource_cache<sound> sounds{8 * 1024 * 1024};

    font default_font { "resources/default.ttf" };
};

#endif //ZENGINE_RESOURCES_HPP
