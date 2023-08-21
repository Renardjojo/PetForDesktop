#pragma once

#include "Engine/WindowGLFW.hpp"

#include <glad/glad.h>

class Window : public WindowGLFW
{
protected:
    void initGraphicAPI();

public:
    void init(struct GameData& datas);

    void initDrawContext();

    void setSize(const Vec2 windowSize) noexcept
    {
        if (m_size == windowSize)
            return;

        WindowGLFW::setSize(windowSize);
    }
};