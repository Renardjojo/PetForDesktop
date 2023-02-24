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
    SpriteAnimator& spriteAnimator;
    SpriteSheet&    spriteSheets;
    int             frameRate;
    bool            loop;

public:
    AnimationNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop = true)
        : spriteAnimator{inSpriteAnimator}, spriteSheets{inSpriteSheets}, frameRate{inFrameRate}, loop{inLoop}
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
    PetJumpNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, Vec2 inBaseDir,
                float inVThrust, float inHThrust)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, false), baseDir{inBaseDir}, vThrust{inVThrust},
          hThrust{inHThrust}
    {
    }

    void onUpdate(GameData& blackBoard, double dt) override
    {
        AnimationNode::onUpdate(blackBoard, dt);

        if (spriteAnimator.isDone()) // Enter only for jump begin because don't loop.
        {
            blackBoard.velocity += baseDir * (blackBoard.side * 2.f - 1.f) * hThrust - blackBoard.gravity * vThrust;
            blackBoard.isGrounded = false;
        }
    }
};

class GrabNode : public AnimationNode
{
public:
    GrabNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop)
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        AnimationNode::onEnter(blackBoard);
        blackBoard.isGrab = true;
    }

    void onExit(GameData& blackBoard) override
    {
        AnimationNode::onExit(blackBoard);
        blackBoard.isGrab = false;
    }
};

class MovementDirectionNode : public AnimationNode
{
    std::vector<Vec2> directions;
    Vec2              baseDir;
    bool              applyGravity;

public:
    MovementDirectionNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate,
                          std::vector<Vec2> inDir, bool inApplyGravity = true, bool inLoop = true)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop), directions{inDir}, applyGravity{inApplyGravity}
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        AnimationNode::onEnter(blackBoard);
        baseDir         = directions[randNum(0, directions.size() - 1)];
        blackBoard.side = baseDir.dot(Vec2::right()) > 0.f;
        blackBoard.applyGravity = applyGravity;
        blackBoard.continuousVelocity += baseDir;
    }

    void onExit(GameData& blackBoard) override
    {
        AnimationNode::onExit(blackBoard);
        blackBoard.applyGravity = true;
        blackBoard.continuousVelocity -= baseDir;
    }
};