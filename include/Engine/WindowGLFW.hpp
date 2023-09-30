#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/Canvas.hpp"

#include <GLFW/glfw3.h>

void dropCallback(GLFWwindow* window, int count, const char** paths);
void cursorPositionCallback(GLFWwindow* window, double x, double y);
void mousButtonCallBack(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

class WindowGLFW : public Canvas
{
protected:
    GLFWwindow* window = nullptr;

    bool isMousePassThrough;
    bool isForwardWindow;
    bool useMousePassThrough;

protected:
    void initGLFW();
    void preSetupWindow(const struct GameData& datas);
    void postSetupWindow(struct GameData& datas);
    void initWindow(struct GameData& datas);

    void UpdatePositionSize(const Rect& other)
    {
        if (encapsulate(other))
        {
            glfwSetWindowSize(window, m_size.x, m_size.y);
            glfwSetWindowPos(window, m_position.x, m_position.y);
        }
    }

public:
    GETTER_BY_VALUE(Window, window)

    void setMousePassThrough(bool flag)
    {
        if (useMousePassThrough && isMousePassThrough == flag)
            return;

        isMousePassThrough = flag;
        glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, isMousePassThrough);
    }

    void setForwardWindow(bool flag)
    {
        if (isForwardWindow && isForwardWindow == flag)
            return;

        isForwardWindow = flag;
        glfwSetWindowAttrib(window, GLFW_FLOATING, isForwardWindow);
    }

    void setSize(const Vec2 windowSize) noexcept
    {
        if (windowSize == m_size)
            return;
        Canvas::setSize(windowSize);
        glfwSetWindowSize(window, windowSize.x, windowSize.y);
    }

    void setPosition(const Vec2 windowPos) noexcept
    {
        if (windowPos == m_position)
            return;

        Canvas::setPosition(windowPos);
        glfwSetWindowPos(window, windowPos.x, windowPos.y);
    }

    void setPositionSize(const Vec2 windowPos, const Vec2 windowSize) noexcept
    {
        setPosition(windowPos);
        setSize(windowPos);
    }

    inline bool shouldClose() const noexcept
    {
        return glfwWindowShouldClose(window);
    }

    ~WindowGLFW()
    {
        glfwTerminate();
    }

    void initDrawContext()
    {

    }

    void renderFrame()
    {
        glfwSwapBuffers(window);
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