#pragma once

#include "Engine/Vector2.hpp"

#include <GLFW/glfw3.h>

#include <vector>

class Monitors
{
protected:
    std::vector<GLFWmonitor*> monitors;

private:
    static Monitors* s_instances;

public:
    static Monitors& getInstance()
    {
        return *s_instances;
    }

public:
    void init()
    {
        s_instances = this;

        int           monitorCount;
        GLFWmonitor** pMonitors = glfwGetMonitors(&monitorCount);
        monitors.reserve(monitorCount);

        for (int i = 0; i < monitorCount; ++i)
        {
            addMonitor(pMonitors[i]);
        }
    }

    void getMainMonitorWorkingArea(Vec2i& position, Vec2i& size) const
    {
        getMonitorPosition(0, position);
        getMonitorSize(0, size);
    }

    Vec2i getMonitorsSize() const
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
    
    void getMonitorPosition(int index, Vec2i& position) const
    {
        glfwGetMonitorPos(monitors[index], &position.x, &position.y);
    }
    
    void getMonitorSize(int index, Vec2i& size) const
    {
        const GLFWvidmode* currentVideoMode;
        currentVideoMode = glfwGetVideoMode(monitors[index]);
        size.x = currentVideoMode->width;
        size.y = currentVideoMode->height;
    }

    Vec2i getMonitorPhysicalSize() const
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

    int getMonitorsCount() const
    {
        return monitors.size();
    }
};

inline void setMonitorCallback(GLFWmonitor* monitor, int event)
{
    switch (event)
    {
    case GLFW_CONNECTED:
        Monitors::getInstance().addMonitor(monitor);
        break;

    case GLFW_DISCONNECTED:
        Monitors::getInstance().removeMonitor(monitor);
        break;
    }
}