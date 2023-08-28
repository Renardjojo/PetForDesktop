#pragma once

#include "Game/GameData.hpp"
#include "Game/UIMenu.hpp"

class ContextualMenu : public UIMenu
{
protected:
    class Pet& pet;

public:

    ContextualMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition);

    virtual ~ContextualMenu();

    void update(double deltaTime);
};
