#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Vector2.hpp"

#include <GLFW/glfw3.h>

void cursorPositionCallback(GLFWwindow* window, double x, double y);
void mousButtonCallBack(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

class WindowGLFW
{
protected:
    GLFWwindow* window = nullptr;

    Vec2i windowSize = {0, 0};
    Vec2i windowPos  = {0, 0};

protected:
    void initGLFW();
    void preSetupWindow(const struct GameData& datas);
    void postSetupWindow(struct GameData& datas);
    void initWindow(struct GameData& datas);

public:
    GETTER_BY_VALUE(Window, window)
    GETTER_BY_VALUE(Size, windowSize)
    GETTER_BY_VALUE(Pos, windowPos)

    void setSize(const Vec2i in_windowSize) noexcept
    {       
        windowSize = in_windowSize;
        glfwSetWindowSize(window, windowSize.x, windowSize.y);
    }

    void setPos(const Vec2i in_windowPos) noexcept
    {
        windowPos = in_windowPos;
        glfwSetWindowPos(window, windowPos.x, windowPos.y);
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
};