#pragma once

#include <functional>
#include <vector>

#include "Engine/ClassUtility.hpp"
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
    GETTER_BY_VALUE(ID, ID)
    GETTER_BY_VALUE(Width, width)
    GETTER_BY_VALUE(Height, height)
    GETTER_BY_VALUE(ChannelsCount, nbChannels)

    Texture(const char* srcPath, std::function<void()> setupCallback = defaultSetupCallBack);

    Texture(void* data, int pxlWidth, int pxlHeight, int channels = 3,
            std ::function<void()> setupCallback = defaultSetupCallBack);

    Texture(int pxlWidth, int pxlHeight, int channels = 4, std::function<void()> setupCallback = defaultSetupCallBack);

    static void defaultSetupCallBack()
    {

    }

    ~Texture();

    bool isPixelOpaque(Vec2i cursorPos) const
    {        
        return *(data + (cursorPos.x + (height - 1 - cursorPos.y) * width) * sizeof(unsigned char) * nbChannels +
                 3 * sizeof(unsigned char)) > 0;
    }

    void use() const
    {
    }

    //GLenum getChanelEnum()
    //{
    //    return nbChannels > 3 ? GL_RGBA : nbChannels == 3 ? GL_RGB : nbChannels == 2 ? GL_RG : GL_RED;
    //}

    // Warning, texture need to be binding before
    void getPixels(std::vector<unsigned char>& data)
    {
        int pixelsCount = width * height * nbChannels;
        data.reserve(pixelsCount);

        for (size_t i = 0; i < pixelsCount; i++)
        {
            data.emplace_back(0);
        }

    }
};