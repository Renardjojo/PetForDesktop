#pragma once

#ifdef USE_OPENGL_API
#include "Engine/Graphics/ShaderOGL.hpp"
#endif // USE_OPENGL_API

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"
#include "Game/GameData.hpp"

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

    GETTER_BY_VALUE(TileCount, tileCount)
    GETTER_BY_VALUE(SizeFactor, sizeFactor)

    void useSection(GameData& data, Shader& shader, int idSection, bool hFlip = false)
    {
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
};