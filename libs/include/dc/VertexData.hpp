#pragma once
#include <glm/glm.hpp>

namespace dc
{
    struct VertexData
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };
}