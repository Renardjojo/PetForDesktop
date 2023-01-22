#pragma once

#include "Engine/SpriteSheet.hpp"
#include "Game/GameData.hpp"

class SpriteAnimator
{
protected:
    SpriteSheet* pSheet                 = nullptr;
    float        timer                  = 0.f;
    float        maxTimer               = 0.f;
    bool         loop                   = false;
    bool         isEnd                  = false;
    int          frameRate              = 0;
    int          indexCurrentAnimSprite = 0;

public:
    void play(GameData& data, SpriteSheet& inSheet, bool inLoop, int inFrameRate)
    {
        pSheet                 = &inSheet;
        loop                   = inLoop;
        frameRate              = inFrameRate;
        indexCurrentAnimSprite = 0;
        timer                  = 0.f;
        maxTimer               = pSheet->getTileCount() / (float)frameRate;
        isEnd                  = false;
        data.shouldUpdateFrame = true;
    }

    void update(GameData& data, double deltaTime)
    {
        if (!isDone())
        {
            timer += (float)deltaTime;

            while (timer >= maxTimer)
            {
                if (loop)
                {
                    timer -= maxTimer;
                }
                else
                {
                    timer = 0.f;
                    isEnd = true;
                }
            }

            if (indexCurrentAnimSprite != (int)(timer * frameRate))
            {
                data.shouldUpdateFrame = true;
                indexCurrentAnimSprite = static_cast<int>(timer * frameRate);
            }
        }
    }

    void draw(GameData& datas, Shader& shader, bool donthFlip)
    {
        if (pSheet != nullptr)
            pSheet->useSection(datas, shader, indexCurrentAnimSprite, !donthFlip);
    }

    bool isDone() const
    {
        return pSheet == nullptr || isEnd;
    }

    bool isMouseOver(Vec2i cursorPos)
    {
        // Assuming square image
        cursorPos.x += indexCurrentAnimSprite * pSheet->getHeight();
        return pSheet != nullptr ? pSheet->isPixelOpaque(cursorPos) : false;
    }
};