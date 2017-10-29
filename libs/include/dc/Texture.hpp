#pragma once
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <iostream>

namespace dc
{
    enum TextureWrap
    {
        ClampToBorder,
        ClampToEdge,
        Repeat
    };

    enum TextureFilter
    {
        Linear,
        Nearest
    };

    enum TextureFormat
    {
        RGB8,
        RGBA8,
        Depth24,
        Depth24Stencil8
    };

    class Texture
    {
    public:
        Texture(unsigned width, unsigned height, TextureFormat format, TextureWrap wrap, TextureFilter filter)
            : m_width(width), m_height(height)
        {
            glGenTextures(1, &m_id);
            bind();
            setParameters(wrap, filter);
            GLint internalFormats[] = {GL_RGB8, GL_RGBA8, GL_DEPTH_COMPONENT24, GL_DEPTH24_STENCIL8};
            GLint formats[] = {GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL};
            GLenum types[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT};
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[format], m_width, m_height, 0, formats[format], types[format], NULL);
            unbind();
        }

        Texture(std::string path, TextureWrap wrap, TextureFilter filter)
        {
            int width, height, channels;
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
            if (!data)
            {
                std::cout << "failed to load texture at " << path << std::endl;
                stbi_image_free(data);
                return;
            }
            glGenTextures(1, &m_id);
            bind();
            setParameters(wrap, filter);
            GLenum format;
            if (channels == 1)
            {
                format = GL_RED;
            }
            else if (channels == 3)
            {
                format = GL_RGB;
            }
            else if (channels == 4)
            {
                format = GL_RGBA;
            }
            m_width = width;
            m_height = height;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            unbind();

            stbi_image_free(data);
        }

        ~Texture()
        {
            glDeleteTextures(1, &m_id);
        }

        Texture(const Texture& other) = delete;
        Texture& operator=(const Texture& other) = delete;

        void bind() const
        {
            glBindTexture(GL_TEXTURE_2D, m_id);
        }

        void unbind() const
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint id() const { return m_id; }

        void setParameters(TextureWrap wrap, TextureFilter filter)
        {
            GLint wraps[] = {GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE, GL_REPEAT};
            GLint filters[] = {GL_LINEAR, GL_NEAREST};
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps[wrap]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wraps[wrap]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filters[filter]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filters[filter]);
        }

    private:
        GLuint m_id;
        unsigned m_width;
        unsigned m_height;
    };
}