#pragma once

#include <list>
#include <memory>

#include "Engine/ClassUtility.hpp"
#include "Engine/Rect.hpp"
#include "Engine/Vector2.hpp"

class Canvas : public Rect
{
protected:
    std::list<std::shared_ptr<Rect>> m_elements;

public:
    void addElement(Rect& element)
    {
        m_elements.emplace_back(&element);
        element.setOnChange([&](const Rect& other) { encapsulate(other); });
    }
};