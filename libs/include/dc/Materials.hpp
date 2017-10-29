#pragma once
#include <glm/glm.hpp>

#include <string>

namespace dc
{
    struct ObjMaterial
    {
        glm::vec3 Ka;
        glm::vec3 Kd;
        glm::vec3 Ks;
        std::string map_Kd;
    };

    struct IndexGroup
    {
        unsigned offset;
        unsigned count;
        dc::ObjMaterial material;
    };
}