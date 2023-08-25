#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/Rect.hpp"

#include "Game/GameData.hpp"

class SettingMenu : public Rect
{
protected:
    GameData& datas;
    class Pet& pet;
    bool      shouldClose        = false;
    bool      shouldInitPosition = false;

    InteractionComponent interactionComponent;

public:
    GETTER_BY_VALUE(ShouldClose, shouldClose)

    SettingMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition);

    virtual ~SettingMenu();

    void update(double deltaTime);
};
