#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Graphics/WindowOGL.hpp"
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
        bool shouldMousePassThough = true;
        for (auto&& comp : m_components)
        {
            comp->isLeftPressOver    = false;
            comp->isLeftReleaseOver  = false;
            comp->isRightPressOver   = false;
            comp->isRightReleaseOver = false;
            comp->isMouseOver        = comp->getRect().isPointInside(data.window->getPosition(), data.cursorPos);
            if (comp->isMouseOver)
            {
                shouldMousePassThough = false;

                if (comp->onMouseOver != nullptr)
                    comp->onMouseOver();
            }

            if (comp->isMouseOver && data.leftButtonEvent == GLFW_PRESS)
            {
                comp->wasLeftClictSelected = true;
                comp->isLeftPressOver      = true;

                if (comp->onLeftPressOver != nullptr)
                    comp->onLeftPressOver();
            }
            else if (comp->wasLeftClictSelected && data.leftButtonEvent == GLFW_RELEASE)
            {
                comp->wasLeftClictSelected = false;

                if (comp->isMouseOver)
                {
                    comp->isLeftReleaseOver = true;
                    if (comp->onLeftReleaseOver != nullptr)
                        comp->onLeftReleaseOver();
                }
            }

            if (comp->isMouseOver && data.rightButtonEvent == GLFW_PRESS)
            {
                comp->wasRightClictSelected = true;
                comp->isRightPressOver      = true;

                if (comp->onRightPressOver != nullptr)
                    comp->onRightPressOver();
            }
            else if (comp->wasRightClictSelected && data.rightButtonEvent == GLFW_RELEASE)
            {
                comp->wasRightClictSelected = false;

                if (comp->isMouseOver)
                { 
                    comp->isRightReleaseOver = true;
                    if (comp->onRightReleaseOver != nullptr)
                        comp->onRightReleaseOver();
                }
            }
        }

        data.window->setMousePassThrough(shouldMousePassThough);
    }
};