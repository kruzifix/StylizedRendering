#pragma once
#include <vector>

#include "Materials.hpp"
#include "VertexData.hpp"
#include "Shader.hpp"

namespace dc
{
    class Mesh
    {
    public:
        Mesh(std::vector<dc::VertexData>& p_vertices, std::vector<unsigned>& p_indices, std::vector<dc::IndexGroup>& p_groups)
        {
            m_vertices = p_vertices;
            m_indices = p_indices;
            m_groups = p_groups;

            uploadToGPU();
        }

        ~Mesh()
        {
            glDeleteVertexArrays(1, std::addressof(m_vaoId));
        }

        Mesh(const Mesh& other) = delete;
        Mesh& operator=(const Mesh& other) = delete;

        void draw(const dc::Shader& shader) const
        {
            glBindVertexArray(m_vaoId);
            for (const auto& it : m_groups)
            {
                shader.setVec3("Ka", it.material.Ka);
                shader.setVec3("Kd", it.material.Kd);
                shader.setVec3("Ks", it.material.Ks);

                glDrawElements(GL_TRIANGLES, it.count, GL_UNSIGNED_INT, reinterpret_cast<const void*>(sizeof(unsigned) * it.offset));
            }
            glBindVertexArray(0);
        }

    private:
        std::vector<dc::VertexData> m_vertices;
        std::vector<unsigned int> m_indices;
        std::vector<dc::IndexGroup> m_groups;

        GLuint m_vaoId;

        void uploadToGPU()
        {
            glGenVertexArrays(1, std::addressof(m_vaoId));

            GLuint vboId, eboId;
            glGenBuffers(1, std::addressof(vboId));
            glGenBuffers(1, std::addressof(eboId));

            glBindVertexArray(m_vaoId);
            glBindBuffer(GL_ARRAY_BUFFER, vboId);
            glBufferData(GL_ARRAY_BUFFER, sizeof(dc::VertexData) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

            // position = 0
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(dc::VertexData), reinterpret_cast<const void*>(offsetof(dc::VertexData, position)));
            // normal = 1
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(dc::VertexData), reinterpret_cast<const void*>(offsetof(dc::VertexData, normal)));
            // texcoord = 2
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(dc::VertexData), reinterpret_cast<const void*>(offsetof(dc::VertexData, texcoord)));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glDeleteBuffers(1, std::addressof(vboId));
            glDeleteBuffers(1, std::addressof(eboId));
        }
    };
}