#pragma once

#include "Engine/TextureOGL.hpp"
#include "Engine/Log.hpp"

#include <glad/glad.h>

class Framebuffer
{
protected:
    unsigned int ID;

public:
    Framebuffer()
    {
        glGenFramebuffers(1, &ID);
    }

    ~Framebuffer()
    {
        glDeleteFramebuffers(1, &ID);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    void attachTexture(const Texture& texture)
    {
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getID(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            log("Framebuffer error");
        }
    }

    static void bindScreen()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};
