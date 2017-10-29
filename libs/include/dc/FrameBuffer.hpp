#pragma once
#include <glad/glad.h>

#include <vector>

#include "Texture.hpp"

namespace dc
{
    enum FBAttachmentType
    {
        AttachColor,
        AttachDepth
    };

    class FrameBuffer
    {
    public:
        struct AttachmentDef
        {
            FBAttachmentType attachment;
            dc::TextureFormat format;
        };

        FrameBuffer(unsigned width, unsigned height, std::vector<AttachmentDef> attachments)
            : m_width(width), m_height(height)
        {
            glGenFramebuffers(1, &m_id);
            bind();

            unsigned currentColorAttachment = 0;
            for (const auto& it : attachments)
            {
                Texture* texture = new Texture(m_width, m_height, it.format, dc::TextureWrap::ClampToBorder, dc::TextureFilter::Linear);
                m_textures.push_back(texture);

                GLenum attachments[] = {GL_COLOR_ATTACHMENT0 + currentColorAttachment, GL_DEPTH_ATTACHMENT};
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[it.attachment], GL_TEXTURE_2D, texture->id(), 0);

                if (it.attachment == FBAttachmentType::AttachColor)
                {
                    m_drawBuffers.push_back(attachments[0]);
                    ++currentColorAttachment;
                }
            }

            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                throw std::exception("frame buffer incomplete!");
            }

            unbind();
        }

        ~FrameBuffer()
        {
            for (auto& it : m_textures)
            {
                delete it;
            }
            glDeleteFramebuffers(1, &m_id);
        }

        FrameBuffer(const FrameBuffer& other) = delete;
        FrameBuffer& operator=(const FrameBuffer& other) = delete;

        unsigned width() const { return m_width; }
        unsigned height() const { return m_height; }

        void bind() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_id);
            glDrawBuffers(m_drawBuffers.size(), m_drawBuffers.data());
        }

        void unbind() const
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
        }

        const Texture* texture(unsigned index) const
        {
            return m_textures[index];
        }

    private:
        unsigned m_width;
        unsigned m_height;
        GLuint m_id;

        std::vector<GLenum> m_drawBuffers;
        std::vector<Texture*> m_textures;
    };
}