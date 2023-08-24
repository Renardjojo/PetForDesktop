#include "Engine/WindowGLFW.hpp"

#include "Game/GameData.hpp"
#include "Engine/Log.hpp"
#include "Engine/Graphics/WindowOGL.hpp"
#include "Game/Pet.hpp"

void WindowGLFW::initGLFW()
{
    // initialize the library
    if (!glfwInit())
        errorAndExit("glfw initialization error");
}

void WindowGLFW::preSetupWindow(const GameData& datas)
{
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, !datas.showFrameBufferBackground);
    glfwWindowHint(GLFW_VISIBLE, datas.showFrameBufferBackground);
    glfwWindowHint(GLFW_FLOATING, datas.useForwardWindow);

    // Disable depth and stencil buffers
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
}

void WindowGLFW::postSetupWindow(GameData& datas)
{
    useMousePassThrough = datas.useMousePassThoughWindow;
    isMousePassThrough = true;
    glfwSetWindowAttrib(datas.window->getWindow(), GLFW_MOUSE_PASSTHROUGH, isMousePassThrough);
    glfwSetWindowAttrib(datas.window->getWindow(), GLFW_TRANSPARENT_FRAMEBUFFER, true);
    glfwSetWindowAttrib(window, GLFW_DECORATED, datas.showWindow);
    glfwSetWindowAttrib(window, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwSetWindowUserPointer(window, &datas);
    glfwSetMouseButtonCallback(window, mousButtonCallBack);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    glfwDefaultWindowHints();
}

void WindowGLFW::initWindow(GameData& datas)
{
    preSetupWindow(datas);

    m_size = {1.f, 1.f};
    window = glfwCreateWindow(m_size.x, m_size.y, PROJECT_NAME, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        errorAndExit("Create Window error");
    }

    glfwMakeContextCurrent(window);
    postSetupWindow(datas);

    glfwShowWindow(window);

    glfwSetWindowPos(window, m_position.x, m_position.y);
}

void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));
    if (datas.leftButtonEvent == GLFW_PRESS)
    {
        float globalScreenPosX = static_cast<float>(datas.window->getPosition().x + x);
        float globalScreenPosY = static_cast<float>(datas.window->getPosition().y + y);
        datas.deltaCursorPosX += globalScreenPosX - datas.prevCursorPosX;
        datas.deltaCursorPosY += globalScreenPosY - datas.prevCursorPosY;
        datas.prevCursorPosX = globalScreenPosX;
        datas.prevCursorPosY = globalScreenPosY;
        Vec2 delta(datas.deltaCursorPosX, datas.deltaCursorPosY);
        datas.deltasCursorPosBuffer.emplace(datas.timeAcc, delta);
        datas.deltaCursorAcc += delta;
    }
}

void mousButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));
   
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        datas.leftButtonEvent = action;
   
        switch (action)
        {
        case GLFW_PRESS: {
            datas.prevCursorPosX  = static_cast<float>(datas.window->getPosition().x + datas.cursorPos.x);
            datas.prevCursorPosY  = static_cast<float>(datas.window->getPosition().y + datas.cursorPos.y);
            datas.deltaCursorPosX = 0.f;
            datas.deltaCursorPosY = 0.f;

            //TODO interaction component
            for (auto pet : datas.pets)
            {
                pet->getPhysicComponent().isGrounded = false;
            }
            break;
        }
        case GLFW_RELEASE:
            // TODO interaction component
            for (auto pet : datas.pets)
            {
                pet->getPhysicComponent().velocity =
                    datas.deltaCursorAcc / datas.coyoteTimeCursorPos / datas.pixelPerMeter * datas.releaseImpulse;
            }
            break;
        default:
            break;
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        datas.rightButtonEvent = action;
        break;
    default:
        break;
    }
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    // Need always capture the mouse position to trigger the pass through
    double cursPosX, cursPosY;
    glfwGetCursorPos(window, &cursPosX, &cursPosY);
    datas.cursorPos = {static_cast<int>(floor(cursPosX)), static_cast<int>(floor(cursPosY))};
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}