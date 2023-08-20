#pragma once

#include <algorithm>
#include <functional>

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"

class Rect
{
    Vec2 m_position = {0, 0}; // Left up corner
    Vec2 m_size     = {0, 0}; // right bottom corner

    std::function<void(const Rect&)> m_onChange;

public:
    SETTER_BY_CONST_REF(OnChange, m_onChange)

    inline Vec2 getCornerMin() const noexcept
    {
        return m_position;
    }

    inline Vec2 getCornerMax() const noexcept
    {
        return m_position + m_size;
    }

    inline void setCornerMin(Vec2 cornerMin) noexcept
    {
        m_size += m_position - cornerMin;
        m_position = cornerMin;
        m_onChange(*this);
    }

    inline void setCornerMax(Vec2 cornerMax) noexcept
    {
        m_size = m_position - cornerMax;
        m_onChange(*this);
    }

    inline void setPosition(Vec2 position) noexcept
    {
        m_position = position;
        m_onChange(*this);
    }

    inline Vec2 getPosition() const noexcept
    {
        return m_position;
    }

    inline void setSize(Vec2 size) noexcept
    {
        m_size = size;
        m_onChange(*this);
    }

    inline Vec2 getSize() const noexcept
    {
        return m_size;
    }

    void encapsulate(const Rect& other)
    {
        m_position.x = std::min(m_position.x, other.getPosition().x);
        m_position.y = std::min(m_position.y, other.getPosition().y);

        m_size.x = std::max(getCornerMax().x, other.getCornerMax().x) - m_position.x;
        m_size.y = std::max(getCornerMax().y, other.getCornerMax().y) - m_position.y;
    }
};