#pragma once

#include "Engine/StateMachine.hpp"
#include "Engine/SpriteAnimator.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/Utilities.hpp"
#include "Game/GameData.hpp"

class AnimationNode : public StateMachine::Node
{
protected:
    class Pet& pet;
    SpriteAnimator& spriteAnimator;
    SpriteSheet&    spriteSheets;
    int             frameRate;
    bool            loop;

public:
    AnimationNode(Pet& inPet, SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop = true)
        : pet{inPet}, spriteAnimator{inSpriteAnimator}, spriteSheets{inSpriteSheets}, frameRate{inFrameRate},
              loop{inLoop}
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        StateMachine::Node::onEnter(blackBoard);
        spriteAnimator.play(blackBoard, spriteSheets, loop, frameRate);
    }

    void onUpdate(GameData& blackBoard, double dt) override
    {
        StateMachine::Node::onUpdate(blackBoard, dt);
        spriteAnimator.update(blackBoard, dt);
    }

    void onExit(GameData& blackBoard) override
    {
        StateMachine::Node::onExit(blackBoard);
    }

    bool IsAnimationDone()
    {
        return spriteAnimator.isDone();
    }
};

class PetJumpNode : public AnimationNode
{
    Vec2  baseDir = {0.f, 0.f};
    float vThrust = 0.f;
    float hThrust = 0.f;

public:
    PetJumpNode(Pet& inPet, SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate,
                Vec2 inBaseDir,
                float inVThrust, float inHThrust)
        : AnimationNode(inPet, inSpriteAnimator, inSpriteSheets, inFrameRate, false), baseDir{inBaseDir},
          vThrust{inVThrust},
          hThrust{inHThrust}
    {
    }

    void onUpdate(GameData& blackBoard, double dt) override;
};

class GrabNode : public AnimationNode
{
public:
    GrabNode(Pet& inPet, SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop)
        : AnimationNode(inPet, inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop)
    {
    }

    void onEnter(GameData& blackBoard) override;
    void onExit(GameData& blackBoard) override;
};

class MovementDirectionNode : public AnimationNode
{
    std::vector<Vec2> directions;
    Vec2              baseDir;
    bool              applyGravity;

public:
    MovementDirectionNode(Pet& inPet, SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate,
                          std::vector<Vec2> inDir, bool inApplyGravity = true, bool inLoop = true)
        : AnimationNode(inPet, inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop), directions{inDir},
          applyGravity{inApplyGravity}
    {
    }

    void onEnter(GameData& blackBoard) override;
    void onExit(GameData& blackBoard) override;
};