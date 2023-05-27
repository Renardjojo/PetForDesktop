#pragma once

#include "Engine/Shader.hpp"
#include "Engine/Vector2.hpp"
#include "Game/GameData.hpp"

#include <glad/glad.h>

class SpriteSheet : public Texture
{
protected:
    int   tileCount;
    float sizeFactor = 1.f;

public:
    SpriteSheet(const char* srcPath, int inTileCount, float inSizeFactor)
        : Texture(srcPath), tileCount{inTileCount}, sizeFactor{inSizeFactor}
    {
    }

    void useSection(GameData& data, Shader& shader, int idSection, bool hFlip = false)
    {
        data.petSize.x = width / tileCount * data.scale * sizeFactor;
        data.petSize.y = height * data.scale * sizeFactor;
        Vec2i windowSize{data.petSize.x + data.windowExt.x + data.windowMinExt.x,
                         data.petSize.y + data.windowExt.y + data.windowMinExt.y};
        data.window.setSize(windowSize);

        float       hScale  = 1.f / tileCount;
        const float vScale  = 1.f; // This field can be used
        float       hOffSet = idSection / (float)tileCount;
        const float vOffset = 0.f; // This field can be used

        Vec2 clipSpacePos =
            Vec2::remap(static_cast<Vec2i>(data.petPos), data.window.getPos(),
                        data.window.getPos() + data.window.getSize(), Vec2{0, 1}, Vec2{1, 0}); // [-1, 1]
        Vec2 clipSpaceSize =
            Vec2::remap(data.petSize, Vec2{0, 0}, data.window.getSize(), Vec2{0, 0}, Vec2{1, 1}); // [0, 1]

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

    float getSizeFactor() const
    {
        return sizeFactor;
    }

    int getTileCount() const
    {
        return tileCount;
    }
};