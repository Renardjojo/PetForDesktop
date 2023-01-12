#pragma once

#include "Game/GameData.hpp"
#include "Engine/Vector2.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));
    if (datas.leftButtonEvent == GLFW_PRESS)
    {
        datas.deltaCursorPosX = static_cast<float>(x) - datas.prevCursorPosX;
        datas.deltaCursorPosY = static_cast<float>(y) - datas.prevCursorPosY;
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
        case GLFW_PRESS:
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            datas.prevCursorPosX  = static_cast<float>(floor(x));
            datas.prevCursorPosY  = static_cast<float>(floor(y));
            datas.deltaCursorPosX = 0.f;
            datas.deltaCursorPosY = 0.f;
            datas.isGrounded      = false;
            break;
        case GLFW_RELEASE:
            datas.velocity = datas.deltaCursorAcc / datas.coyoteTimeCursorPos / datas.pixelPerMeter * datas.releaseImpulse;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void processMousePassTHoughWindow(GLFWwindow* window, GameData& datas)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    const Vec2 localWinPos             = datas.petPos - datas.windowPos;
    const bool isCursorInsidePetWindow = xPos > localWinPos.x && yPos > localWinPos.y &&
                                         xPos < localWinPos.x + (float)datas.petSize.x &&
                                         yPos < localWinPos.y + (float)datas.petSize.y;

    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, !isCursorInsidePetWindow);
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (datas.useMousePassThoughWindow)
        processMousePassTHoughWindow(window, datas);
}