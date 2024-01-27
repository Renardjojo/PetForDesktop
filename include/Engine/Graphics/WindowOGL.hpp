#pragma once

#include "Engine/WindowSDL.hpp"

#include <glad/glad.h>

class Window : public WindowSDL
{
public:
    void init(struct GameData& datas);

    void initDrawContext();

    void setSize(const Vec2 windowSize) noexcept
    {
        if (m_size == windowSize)
            return;

        WindowSDL::setSize(windowSize);
    }
};