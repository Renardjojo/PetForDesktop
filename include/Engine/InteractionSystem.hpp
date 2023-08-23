#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/Rect.hpp"
#include "Engine/Vector2.hpp"
#include "Game/GameData.hpp"

#include <list>

class InteractionSystem
{
protected:
    std::list<InteractionComponent*> m_components;

public:
    void addComponent(InteractionComponent& comp)
    {
        m_components.emplace_back(&comp);
    }

    void removeComponent(InteractionComponent& comp)
    {
        m_components.remove_if([&](auto inComp) { return inComp == &comp; });
    }

    void update(GameData& data)
    {
        for (auto&& comp : m_components)
        {
            bool isMouseOver = false;
            if (comp->onMouseOver != nullptr && comp->getRect().isPointInside(data.window->getPosition(), data.cursorPos))
            {
                isMouseOver = true;
                comp->onMouseOver();
            }

            if (isMouseOver && data.leftButtonEvent == GLFW_PRESS)
            {
                comp->wasLeftClictSelected = true;

                if (comp->onSelectedLeftPress != nullptr)
                    comp->onSelectedLeftPress();
            }
            else if (comp->wasLeftClictSelected && data.leftButtonEvent == GLFW_RELEASE)
            {
                comp->wasLeftClictSelected = false;

                if (isMouseOver && comp->onSelectedLeftRelease != nullptr)
                    comp->onSelectedLeftRelease();
            }

            if (isMouseOver && data.rightButtonEvent == GLFW_PRESS)
            {
                comp->wasRightClictSelected = true;

                if (comp->onSelectedRightPress != nullptr)
                    comp->onSelectedRightPress();
            }
            else if (comp->wasRightClictSelected && data.rightButtonEvent == GLFW_RELEASE)
            {
                comp->wasRightClictSelected = false;

                if (isMouseOver && comp->onSelectedRightRelease != nullptr)
                    comp->onSelectedRightRelease();
            }
        }
    }
};