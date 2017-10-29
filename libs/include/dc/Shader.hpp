#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace dc
{
    enum ShaderStage
    {
        Vertex,
        Geometry,
        Fragment
    };

    class Shader
    {
    public:
        struct ShaderStageDef
        {
            ShaderStage stage;
            std::string path;
        };

        Shader(const std::vector<ShaderStageDef>& stages)
        {
            m_stages = stages;
            reload();
        }

        ~Shader()
        {
            glDeleteProgram(m_id);
        }

        Shader(const Shader& other) = delete;
        Shader& operator=(const Shader& other) = delete;

        void reload()
        {
            std::vector<GLuint> shaderIDs;

            for (const auto& it : m_stages)
            {
                GLuint sid;
                if (!compileShaderSource(it, sid))
                {
                    for (const auto& ids : shaderIDs)
                    {
                        glDeleteShader(ids);
                    }
                    return;
                }
                shaderIDs.push_back(sid);
            }

            GLuint pid = glCreateProgram();
            for (const auto& it : shaderIDs)
            {
                glAttachShader(pid, it);
            }
            glLinkProgram(pid);
            checkErrors(pid, "program");
            for (const auto& ids : shaderIDs)
            {
                glDeleteShader(ids);
            }
            m_id = pid;
        }

        void use() const
        {
            glUseProgram(m_id);
        }

        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
        }

        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
        }

        void setVec3(const std::string& name, const glm::vec3& vec) const
        {
            glUniform3f(glGetUniformLocation(m_id, name.c_str()), vec.x, vec.y, vec.z);
        }

        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, false, glm::value_ptr(mat));
        }

    private:
        std::vector<ShaderStageDef> m_stages;
        GLuint m_id;

        bool compileShaderSource(const ShaderStageDef& ssd, GLuint& sid)
        {
            GLuint shaderTypes[] = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};

            GLuint id = glCreateShader(shaderTypes[ssd.stage]);
            std::string source = loadSource(ssd.path);
            const char* code = source.c_str();
            glShaderSource(id, 1, &code, NULL);
            glCompileShader(id);

            std::string stageNames[] = {"vertex", "geometry", "fragment"};
            if (checkErrors(id, stageNames[ssd.stage]))
            {
                glDeleteShader(id);
                return false;
            }
            sid = id;
            return true;
        }

        std::string loadSource(const std::string& path)
        {
            std::string source;
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                std::stringstream stream;
                file.open(path);
                stream << file.rdbuf();
                file.close();
                return stream.str();
            }
            catch (std::ifstream::failure e)
            {
                throw;
            }
        }

        bool checkErrors(GLuint shader, std::string type)
        {
            int success;
            char infoLog[1024];
            if (type == "program")
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "Error linking program:\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                    return true;
                }
            }
            else
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "Error compiling " << type << " shader:\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                    return true;
                }
            }
            return false;
        }
    };
}