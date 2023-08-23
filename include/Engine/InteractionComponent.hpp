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
    std::function<void()> onSelectedLeftPress;
    std::function<void()> onSelectedLeftRelease;
    std::function<void()> onSelectedRightPress;
    std::function<void()> onSelectedRightRelease;

public:
    DEFAULT_GETTER_SETTER_VALUE(Rect, m_rect)

    InteractionComponent(Rect& rect) : m_rect{rect}
    {
    }
};