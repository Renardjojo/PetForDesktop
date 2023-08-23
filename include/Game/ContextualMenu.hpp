#pragma once

#include <cpr/cpr.h>

#include "Engine/ClassUtility.hpp"
#include "Engine/Rect.hpp"

#include "Game/GameData.hpp"

class ContextualMenu : public Rect
{
protected:
    GameData& datas;
    bool      shouldClose = false;

public:
    GETTER_BY_VALUE(ShouldClose, shouldClose)

    ContextualMenu(GameData& data);

    virtual ~ContextualMenu();

    void update(double deltaTime);

    void textCentered(std::string text);
};
