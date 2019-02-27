//
// Created by fotyev on 2018-07-13.
//

#ifndef ZENGINE_RESOURCES_HPP
#define ZENGINE_RESOURCES_HPP

#include "util/resource_cache.hpp"

#include "opengl/texture.hpp"
#include "opengl/textrender.hpp"

#include "util/semiconfig.hpp"


//=- register_component(class=>'resources_c', name=>'resources', priority=>50);

class resources_c
{
public:
    resource_cache<texture> textures{SCFG(cache.textures, 100 * 1024u)};

    font default_font { "resources/default.ttf" };
};

#endif //ZENGINE_RESOURCES_HPP
