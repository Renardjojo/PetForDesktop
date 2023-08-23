#include "Game/Animations.hpp"
#include "Game/Pet.hpp"

void PetJumpNode::onUpdate(GameData& blackBoard, double dt)
{
    AnimationNode::onUpdate(blackBoard, dt);

    if (spriteAnimator.isDone()) // Enter only for jump begin because don't loop.
    {
        pet.getPhysicComponent().velocity += baseDir * ((int)pet.getSide() * 2.f - 1.f) * hThrust - blackBoard.gravity * vThrust;
        pet.getPhysicComponent().isGrounded = false;
    }
}

void GrabNode::onEnter(GameData& blackBoard)
{
    AnimationNode::onEnter(blackBoard);
    pet.setIsGrab(true);
}

void GrabNode::onExit(GameData& blackBoard)
{
    AnimationNode::onExit(blackBoard);
    pet.setIsGrab(false);
}

void MovementDirectionNode::onEnter(GameData& blackBoard)
{
    AnimationNode::onEnter(blackBoard);
    baseDir = directions[randNum(0, directions.size() - 1)];
    pet.setSide((Pet::ESide)(baseDir.dot(Vec2::right()) > 0.f));
    pet.getPhysicComponent().applyGravity = applyGravity;
    pet.getPhysicComponent().continuousVelocity += baseDir;
}

void MovementDirectionNode::onExit(GameData& blackBoard)
{
    AnimationNode::onExit(blackBoard);
    pet.getPhysicComponent().applyGravity = true;
    pet.getPhysicComponent().continuousVelocity -= baseDir;
}