#pragma once

#include <algorithm>
#include <functional>

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"

class Rect
{
protected:
    Vec2 m_position = {0, 0}; // Left up corner
    Vec2 m_size     = {0, 0}; // right bottom corner

    std::function<void(const Rect&)> m_onChange;

    virtual void onChange()
    {
        if (m_onChange)
            m_onChange(*this);
    }

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
        onChange();
    }

    inline void setCornerMax(Vec2 cornerMax) noexcept
    {
        m_size = m_position - cornerMax;
        onChange();
    }

    inline void setPosition(const Vec2 position) noexcept
    {
        m_position = position;
        onChange();
    }

    inline Vec2 getPosition() const noexcept
    {
        return m_position;
    }

    inline void setSize(Vec2 size) noexcept
    {
        m_size = size;
        onChange();
    }

    inline Vec2 getSize() const noexcept
    {
        return m_size;
    }

    void encapsulate(const Rect& other)
    {
        if (other.getPosition().x < m_position.x)
        {
            m_size.x += m_position.x - other.getPosition().x;
            m_position.x = other.getPosition().x;
        }

        if (other.getPosition().y < m_position.y)
        {
            m_size.y += m_position.y - other.getPosition().y;
            m_position.y = other.getPosition().y;
        }

        m_size.x = std::max(getCornerMax().x, other.getCornerMax().x) - m_position.x;
        m_size.y = std::max(getCornerMax().y, other.getCornerMax().y) - m_position.y;
    }
};