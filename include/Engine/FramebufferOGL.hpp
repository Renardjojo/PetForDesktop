#pragma once

#include "Engine/Texture.hpp"
#include "Engine/Log.hpp"

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
