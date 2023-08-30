#pragma once

#include "Game/UIMenu.hpp"
#include "Game/GameData.hpp"

class SettingMenu : public UIMenu
{
protected:
    class Pet& pet;

public:
    SettingMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition);

    virtual ~SettingMenu();

    void update(double deltaTime);
};
