#pragma once

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
        bool isLeftClickConsumed   = false;
        bool isRightClickConsumed  = false;

        for (int i = m_components.size() - 1; i >= 0; --i)
        {
            auto it = m_components.begin();
            std::advance(it, i);
            InteractionComponent* comp = *it;

            comp->isLeftPressOver  = false;
            comp->isLeftRelease    = false;
            comp->isRightPressOver = false;
            comp->isRightRelease   = false;
            comp->isMouseOver      = comp->getRect().isPointInside(data.window->getPosition(), data.cursorPos);

            if (comp->isMouseOver)
            {
                shouldMousePassThough = false;

                if (comp->onMouseOver != nullptr)
                    comp->onMouseOver();
            }

            if (!isLeftClickConsumed && comp->isMouseOver && data.leftButtonEvent == GLFW_PRESS)
            {
                comp->isLeftSelected  = true;
                comp->isLeftPressOver = true;
                isLeftClickConsumed   = true;

                if (comp->onLeftPressOver != nullptr)
                    comp->onLeftPressOver();
            }
            else if (comp->isLeftSelected && data.leftButtonEvent == GLFW_RELEASE)
            {
                comp->isLeftSelected = false;
                comp->isLeftRelease  = true;

                if (comp->isMouseOver)
                {
                    if (comp->onLeftReleaseOver != nullptr)
                        comp->onLeftReleaseOver();
                }
            }

            if (!isRightClickConsumed && comp->isMouseOver && data.rightButtonEvent == GLFW_PRESS)
            {
                comp->isRightSelected  = true;
                comp->isRightPressOver = true;
                isRightClickConsumed   = true;

                if (comp->onRightPressOver != nullptr)
                    comp->onRightPressOver();
            }
            else if (comp->isRightSelected && data.rightButtonEvent == GLFW_RELEASE)
            {
                comp->isRightSelected = false;
                comp->isRightRelease  = true;

                if (comp->isMouseOver)
                {
                    if (comp->onRightReleaseOver != nullptr)
                        comp->onRightReleaseOver();
                }
            }
        }

        data.window->setMousePassThrough(shouldMousePassThough);
    }
};