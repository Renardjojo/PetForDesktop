#pragma once

#include <glad/glad.h>
#include <functional>
#include <vector>

#include "Engine/Log.hpp"
#include "Engine/Vector2.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
protected:
    unsigned int ID;
    int          width, height;
    int          nbChannels;
    unsigned char* data = nullptr;

public:
    Texture(const char* srcPath, std::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        // load image, create texture and generate mipmaps
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        data = stbi_load(srcPath, &width, &height, &nbChannels, 0);
        if (data)
        {
            if (nbChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            log("Failed to load texture");
        }
    }

    Texture(void* data, int pxlWidth, int pxlHeight, int channels = 3,
            std ::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        width           = pxlWidth;
        height          = pxlHeight;
        nbChannels      = channels;
        GLenum chanEnum = getChanelEnum();
        glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    }

    Texture(int pxlWidth, int pxlHeight, int channels = 4, std::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        width           = pxlWidth;
        height          = pxlHeight;
        nbChannels      = channels;
        GLenum chanEnum = getChanelEnum();

        glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, chanEnum, GL_UNSIGNED_BYTE, 0);
    }

    static void defaultSetupCallBack()
    {
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    ~Texture()
    {
        if (data != nullptr)
            stbi_image_free(data);
        glDeleteTextures(1, &ID);
    }

    bool isPixelOpaque(Vec2i cursorPos) const
    {        
        return *(data + (cursorPos.x + (height - 1 - cursorPos.y) * width) * sizeof(unsigned char) * nbChannels +
                 3 * sizeof(unsigned char)) > 0;
    }

    void use() const
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    int getHeight() const
    {
        return height;
    }

    int getWidth() const
    {
        return width;
    }

    int getChannelCount() const
    {
        return nbChannels;
    }

    int getID() const
    {
        return ID;
    }

    GLenum getChanelEnum()
    {
        return nbChannels > 3 ? GL_RGBA : nbChannels == 3 ? GL_RGB : nbChannels == 2 ? GL_RG : GL_RED;
    }

    // Warning, texture need to be binding before
    void getPixels(std::vector<unsigned char>& data)
    {
        int pixelsCount = width * height * nbChannels;
        data.reserve(pixelsCount);

        for (size_t i = 0; i < pixelsCount; i++)
        {
            data.emplace_back(0);
        }

        glGetTextureImage(ID, 0, getChanelEnum(), GL_UNSIGNED_BYTE, pixelsCount * sizeof(unsigned char), &data[0]);
    }
};