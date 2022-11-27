#pragma once

#include "Game/GameData.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/Shader.hpp"

#include <glad/glad.h>

class SpriteSheet : public Texture
{
protected:
    int tileCount;

public:
    SpriteSheet(const char* srcPath) : Texture(srcPath)
    {
        tileCount = width / height;
    }

    void useSection(GameData& data, Shader& shader, int idSection, bool hFlip = false)
    {
        data.petSize.x    = height * data.scale; // use height because texture is horizontal sprite sheet only
        data.petSize.y    = height * data.scale;
        data.windowSize.x = data.petSize.x + data.windowExt.x + data.windowMinExt.x;
        data.windowSize.y = data.petSize.y + data.windowExt.y + data.windowMinExt.y;
        data.petPosLimit  = {data.videoMode->width - data.petSize.x, data.videoMode->height - data.petSize.y};
        glfwSetWindowSize(data.window, data.windowSize.x, data.windowSize.y);

        float       hScale  = 1.f / tileCount;
        const float vScale  = 1.f; // This field can be used
        float       hOffSet = idSection / (float)tileCount;
        const float vOffset = 0.f; // This field can be used

        Vec2 clipSpacePos  = Vec2::remap(static_cast<Vec2i>(data.petPos), data.windowPos,
                                        data.windowPos + data.windowSize, Vec2{0, 1}, Vec2{1, 0});          // [-1, 1]
        Vec2 clipSpaceSize = Vec2::remap(data.petSize, Vec2{0, 0}, data.windowSize, Vec2{0, 0}, Vec2{1, 1}); // [0, 1]

        // In shader, based on bottom left instead of upper left
        clipSpacePos.y -= clipSpaceSize.y;

        if (hFlip)
        {
            hOffSet += hScale;
            hScale *= -1;
        }

        shader.use();
        shader.setVec4("uScaleOffSet", hScale, vScale, hOffSet, vOffset);
        shader.setVec4("uClipSpacePosSize", clipSpacePos.x, clipSpacePos.y, clipSpaceSize.x, clipSpaceSize.y);
        use();
    }

    int getTileCount() const
    {
        return tileCount;
    }
};