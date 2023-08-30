#pragma once

#include "Engine/StateMachine.hpp"
#include "Game/GameData.hpp"

#include <GLFW/glfw3.h>

struct AnimationEndTransition : public StateMachine::Node::Transition
{
    class Pet& pet;

    AnimationEndTransition(class Pet& inPet) : pet{inPet}
    {
    }

    bool canTransition(GameData& blackBoard) final;
};

struct IsGroundedTransition : public StateMachine::Node::Transition
{
    class Pet& pet;

    IsGroundedTransition(class Pet& inPet) : pet{inPet}
    {
    }

    bool canTransition(GameData& blackBoard) final;
};

struct IsNotGroundedTransition : public StateMachine::Node::Transition
{
    class Pet& pet;

    IsNotGroundedTransition(class Pet& inPet) : pet{inPet}
    {
    }

    bool canTransition(GameData& blackBoard) final;
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
        return timer >= delay;
    };

    void onEnter(GameData& blackBoard) final;

    void onUpdate(GameData& blackBoard, double dt) final
    {
        timer += static_cast<float>(dt);
    }
};

struct StartLeftClicTransition : public StateMachine::Node::Transition
{
    class Pet& pet;

    StartLeftClicTransition(class Pet& inPet) : pet{inPet}
    {
    }

    bool canTransition(GameData& blackBoard) final;
};

struct TouchScreenEdgeTransition : public StateMachine::Node::Transition
{
    class Pet& pet;

    TouchScreenEdgeTransition(class Pet& inPet) : pet{inPet}
    {
    }

    bool canTransition(GameData& blackBoard) final;
};

struct EndLeftClicTransition : public StateMachine::Node::Transition
{
protected:
    bool leftWasPressed = false;
    class Pet& pet;

public:

    EndLeftClicTransition(class Pet& inPet) : pet{inPet}
    {
    }

    void onEnter(GameData& blackBoard) final;

    bool canTransition(GameData& blackBoard) final;
};