#pragma once

#include "Engine/StateMachine.hpp"
#include "Game/GameData.hpp"
#include "Game/Animations.hpp"

#include <GLFW/glfw3.h>

struct AnimationEndTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return static_cast<AnimationNode*>(pOwner)->IsAnimationDone();
    };
};

struct IsGroundedTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return blackBoard.isGrounded;
    };
};

struct IsNotGroundedTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return !blackBoard.isGrounded;
    };
};

struct RandomDelayTransition : public StateMachine::Node::Transition
{
protected:
    float delay = 0.f;
    float timer = 0.f;

    int baseDelay_ms = 0;
    int interval_ms  = 0;

public:
    RandomDelayTransition(int inBaseDelay_ms, int inInterval_ms)
        : baseDelay_ms{inBaseDelay_ms}, interval_ms{inInterval_ms}
    {
    }

    bool canTransition(GameData& blackBoard) final
    {
        if (timer >= delay)
            return true;
        return false;
    };

    void onEnter(GameData& blackBoard) final
    {
        timer = 0.f;
        delay = static_cast<float>(baseDelay_ms + randNum(-interval_ms, interval_ms));
        delay *= 0.001f; // to seconde
    }

    void onUpdate(GameData& blackBoard, double dt) final
    {
        timer += static_cast<float>(dt);
    }
};

struct StartLeftClicTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return blackBoard.leftButtonEvent == GLFW_PRESS;
    };
};

struct TouchScreenEdgeTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return blackBoard.touchScreenEdge;
    };
};

struct EndLeftClicTransition : public StateMachine::Node::Transition
{
protected:
    bool leftWasPressed = false;

public:
    bool canTransition(GameData& blackBoard) final
    {
        if (blackBoard.leftButtonEvent == GLFW_PRESS)
        {
            leftWasPressed = true;
        }

        if (blackBoard.leftButtonEvent != GLFW_PRESS && leftWasPressed)
        {
            leftWasPressed = false;
            return true;
        }
        return false;
    };
};