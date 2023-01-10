#pragma once

#include "Engine/Vector2.hpp"

#include <GLFW/glfw3.h>

#include <vector>

class Monitors
{
protected:
    std::vector<GLFWmonitor*> monitors;

public:
    void init()
    {
        int           monitorCount;
        GLFWmonitor** pMonitors = glfwGetMonitors(&monitorCount);
        monitors.reserve(monitorCount);

        for (int i = 0; i < monitorCount; ++i)
        {
            addMonitor(pMonitors[i]);
            glfwSetMonitorUserPointer(pMonitors[i], this);
        }
    }

    Vec2i getMonitorsSize()
    {
        Vec2i size = Vec2i::zero();
        const GLFWvidmode* currentVideoMode;
        for (int i = 0; i < monitors.size(); i++)
        {
            currentVideoMode = glfwGetVideoMode(monitors[i]);
            size.x += currentVideoMode->width;
            size.y += currentVideoMode->height;
        }
        return size;
    }

    Vec2i getMonitorPhysicalSize()
    {
        Vec2i sizeMM = Vec2i::zero();
        int width_mm, height_mm;
        for (int i = 0; i < monitors.size(); i++)
        {
            glfwGetMonitorPhysicalSize(monitors[i], &width_mm, &height_mm);
            sizeMM.x += width_mm;
            sizeMM.y += height_mm;
        }
        return sizeMM;
    }

    void addMonitor(GLFWmonitor* monitor)
    {
        monitors.emplace_back(monitor);
    }

    void removeMonitor(const GLFWmonitor* monitor)
    {
        for (int i = 0; i < monitors.size(); ++i)
        {
            if (monitors[i] == monitor)
            {
                monitors.erase(monitors.begin() + i);
                break;
            }
        }
    }
};