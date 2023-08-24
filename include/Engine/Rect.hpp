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

    virtual ~Rect()
    {
    }

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
        setPosition(cornerMin);
        onChange();
    }

    inline void setCornerMax(Vec2 cornerMax) noexcept
    {
        setSize(m_position - cornerMax);
        onChange();
    }

    virtual void setPosition(const Vec2 position)
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

    // return true if change;
    bool encapsulate(const Rect& other)
    {
        bool hasChanged = false;

        if (other.getPosition().x < m_position.x)
        {
            m_size.x += m_position.x - other.getPosition().x;
            m_position.x = other.getPosition().x;
            hasChanged   = true;
        }

        if (other.getPosition().y < m_position.y)
        {
            m_size.y += m_position.y - other.getPosition().y;
            m_position.y = other.getPosition().y;
            hasChanged   = true;
        }

        if (other.getCornerMax().x > getCornerMax().x)
        {
            m_size.x   = other.getCornerMax().x - m_position.x;
            hasChanged = true;
        }

        if (other.getCornerMax().y > getCornerMax().y)
        {
            m_size.y   = other.getCornerMax().y - m_position.y;
            hasChanged = true;
        }

        return hasChanged;
    }

    virtual bool isPointInside(Vec2 pointPos)
    {
        return pointPos.x > 0 && pointPos.y > 0 && pointPos.x < m_size.x && pointPos.y < m_size.y;
    }

    bool isPointInside(Vec2 worldPosition, Vec2 pointPos)
    {
        const Vec2 localPos = pointPos - (m_position - worldPosition);
        return isPointInside(localPos);
    }
};