#pragma once

#include "Engine/Texture.hpp"
#include "Engine/Log.hpp"

#include <glad/glad.h>

class Framebuffer
{
protected:
    unsigned int ID;

public:
    Framebuffer()
    {

    }

    ~Framebuffer()
    {

    }

    void bind()
    {

    }

    void attachTexture(const Texture& texture)
    {

    }

    static void bindScreen()
    {
    }
};
