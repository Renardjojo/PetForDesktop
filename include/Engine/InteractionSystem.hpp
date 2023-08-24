#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/Rect.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/Graphics/WindowOGL.hpp"
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
        bool shouldMousePassThough = true;
        for (auto&& comp : m_components)
        {
            bool isMouseOver = comp->getRect().isPointInside(data.window->getPosition(), data.cursorPos);
            if (isMouseOver)
            {
                shouldMousePassThough = false;

                if (comp->onMouseOver != nullptr)
                    comp->onMouseOver();
            }

            if (isMouseOver && data.leftButtonEvent == GLFW_PRESS)
            {
                comp->wasLeftClictSelected = true;

                if (comp->onLeftPressOver != nullptr)
                    comp->onLeftPressOver();
            }
            else if (comp->wasLeftClictSelected && data.leftButtonEvent == GLFW_RELEASE)
            {
                comp->wasLeftClictSelected = false;

                if (isMouseOver && comp->onLeftReleaseOver != nullptr)
                    comp->onLeftReleaseOver();
            }

            if (isMouseOver && data.rightButtonEvent == GLFW_PRESS)
            {
                comp->wasRightClictSelected = true;

                if (comp->onRightPressOver != nullptr)
                    comp->onRightPressOver();
            }
            else if (comp->wasRightClictSelected && data.rightButtonEvent == GLFW_RELEASE)
            {
                comp->wasRightClictSelected = false;

                if (isMouseOver && comp->onRightReleaseOver != nullptr)
                    comp->onRightReleaseOver();
            }
        }

        data.window->setMousePassThrough(shouldMousePassThough);
    }
};