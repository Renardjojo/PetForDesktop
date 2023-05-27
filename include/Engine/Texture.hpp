#pragma once

#include <glad/glad.h>
#include <functional>
#include <vector>

#include "Engine/Log.hpp"
#include "Engine/Vector2.hpp"

class Texture
{
protected:
    unsigned int ID;
    int          width, height;
    int          nbChannels;
    unsigned char* data = nullptr;

public:
    Texture(const char* srcPath, std::function<void()> setupCallback = defaultSetupCallBack);

    Texture(void* data, int pxlWidth, int pxlHeight, int channels = 3,
            std ::function<void()> setupCallback = defaultSetupCallBack);

    Texture(int pxlWidth, int pxlHeight, int channels = 4, std::function<void()> setupCallback = defaultSetupCallBack);

    static void defaultSetupCallBack()
    {
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    ~Texture();

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