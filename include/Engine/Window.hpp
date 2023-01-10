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
            datas.prevCursorPosX  = static_cast<float>(datas.cursorPos.x);
            datas.prevCursorPosY  = static_cast<float>(datas.cursorPos.y);
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

void setMonitorCallback(GLFWmonitor* monitor, int event)
{
    GameData& datas = *static_cast<GameData*>(glfwGetMonitorUserPointer(monitor));

    switch (event)
    {
    case GLFW_CONNECTED:
        datas.monitors.addMonitor(monitor);
        break;

    case GLFW_DISCONNECTED:
        datas.monitors.removeMonitor(monitor);
        break;
    }
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    double cursPosX, cursPosY;
    glfwGetCursorPos(window, &cursPosX, &cursPosY);
    datas.cursorPos = {static_cast<int>(floor(cursPosX)), static_cast<int>(floor(cursPosY))};

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}