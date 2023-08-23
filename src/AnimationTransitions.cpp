#include "Game/AnimationTransitions.hpp"
#include "Engine/Utilities.hpp"
#include "Game/Animations.hpp"
#include "Game/Pet.hpp"

bool IsGroundedTransition::canTransition(GameData& blackBoard)
{
    return pet.getPhysicComponent().isGrounded;
}

bool IsNotGroundedTransition::canTransition(GameData& blackBoard)
{
    return !pet.getPhysicComponent().isGrounded;
}

bool AnimationEndTransition::canTransition(GameData& blackBoard)
{
    return static_cast<AnimationNode*>(pOwner)->IsAnimationDone();
}

void RandomDelayTransition::onEnter(GameData& blackBoard)
{
    timer = 0.f;
    delay = static_cast<float>(baseDelay_ms + randNum(-interval_ms, interval_ms));
    delay *= 0.001f; // to seconde
}

bool TouchScreenEdgeTransition::canTransition(GameData& blackBoard)
{
    return pet.getPhysicComponent().touchScreenEdge;
}

bool EndLeftClicTransition::canTransition(GameData& blackBoard)
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
}