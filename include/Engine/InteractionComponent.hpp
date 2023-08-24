#pragma once

#include "Engine/Rect.hpp"
#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"
#include <functional>

struct InteractionComponent
{
protected:
    Rect& m_rect;

public:
    bool wasLeftClictSelected;
    bool wasRightClictSelected;

    std::function<void()> onMouseOver;
    std::function<void()> onLeftPressOver;
    std::function<void()> onLeftReleaseOver;
    std::function<void()> onRightPressOver;
    std::function<void()> onRightReleaseOver;

public:
    DEFAULT_GETTER_SETTER_VALUE(Rect, m_rect)

    InteractionComponent(Rect& rect) : m_rect{rect}
    {
    }
};