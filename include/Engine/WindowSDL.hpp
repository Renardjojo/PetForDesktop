#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/Canvas.hpp"

#include <SDL3/SDL.h>

class WindowSDL : public Canvas
{
protected:
    SDL_GLContext m_glcontext = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Window*   m_window   = nullptr;

    bool m_isMousePassThrough;
    bool m_isForwardWindow;
    bool m_useMousePassThrough;
    bool m_shouldClose;

protected:
    void init();
    void preSetupWindow(const struct GameData& datas){}
    void postSetupWindow(struct GameData& datas);
    void initWindow(struct GameData& datas);

    void UpdatePositionSize(const Rect& other)
    {
        if (encapsulate(other))
        {
            SDL_SetWindowSize(m_window, m_size.x, m_size.y);
            SDL_SetWindowPosition(m_window, m_position.x, m_position.y);
        }
    }

public:
    GETTER_BY_VALUE(Window, m_window)

    void setMousePassThrough(bool flag)
    {
        if (m_useMousePassThrough && m_isMousePassThrough == flag)
            return;

        m_isMousePassThrough = flag;
        SDL_CaptureMouse(m_isMousePassThrough);
    }

    void setForwardWindow(bool flag)
    {
        if (m_isForwardWindow && m_isForwardWindow == flag)
            return;

        m_isForwardWindow = flag;
        SDL_SetWindowAlwaysOnTop(m_window, m_isForwardWindow);
    }

    void setSize(const Vec2 windowSize) noexcept
    {
        if (windowSize == m_size)
            return;
        Canvas::setSize(windowSize);
        SDL_SetWindowSize(m_window, windowSize.x, windowSize.y);
    }

    void setPosition(const Vec2 windowPos) noexcept
    {
        if (windowPos == m_position)
            return;

        Canvas::setPosition(windowPos);
        SDL_SetWindowPosition(m_window, windowPos.x, windowPos.y);
    }

    void setPositionSize(const Vec2 windowPos, const Vec2 windowSize) noexcept
    {
        setPosition(windowPos);
        setSize(windowPos);
    }

    inline bool shouldClose() const noexcept
    {
        return m_shouldClose;
    }

    ~WindowSDL()
    {
        SDL_GL_DeleteContext(m_glcontext); 
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);

        SDL_Quit();
    }

    void processInput();

    void renderFrame()
    {
        SDL_RenderPresent(m_renderer);
    }

    void addElement(Rect& element)
    {
        m_elements.emplace_back(&element);
        element.setOnChange([&](const Rect& other) 
            { UpdatePositionSize(other); });
    }

    void removeElement(Rect& element)
    {
        m_elements.remove_if([&](auto rect) { return rect == &element; });
    }
};