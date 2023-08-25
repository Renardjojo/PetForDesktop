#pragma once

#include <cpr/cpr.h>

#include "Engine/ClassUtility.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/Rect.hpp"

#include "Game/GameData.hpp"

class ContextualMenu : public Rect
{
protected:
    GameData& datas;
    class Pet& pet;
    bool      shouldClose        = false;
    bool      shouldInitPosition = false;

    InteractionComponent interactionComponent;

public:
    GETTER_BY_VALUE(ShouldClose, shouldClose)

    ContextualMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition);

    virtual ~ContextualMenu();

    void update(double deltaTime);

    void textCentered(std::string text);
};
