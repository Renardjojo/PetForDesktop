#pragma once

#include "Engine/Log.hpp"
#include "Engine/Vector2.hpp"

#include <SDL3/SDL.h>
#include <vector>

class Monitors
{
protected:
    std::vector<SDL_DisplayID> monitors;

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

        int            monitorCount;
        SDL_DisplayID* pMonitors = SDL_GetDisplays(&monitorCount);
        if (pMonitors == NULL)
        {
            errorAndExit(std::string("SDL_GetDisplays error: ") + SDL_GetError());
            return;
        }

        monitors.reserve(monitorCount);

        for (int i = 0; i < monitorCount; ++i)
        {
            addMonitor(pMonitors[i]);
        }
        SDL_free(pMonitors);
    }

    void getMainMonitorWorkingArea(Vec2i& position, Vec2i& size) const
    {
        position = Vec2i::zero();
        size     = Vec2i::zero();

        SDL_Rect      rect;
        SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
        if (SDL_GetDisplayUsableBounds(primaryDisplay, &rect) != 0)
        {
            errorAndExit(std::string("SDL_GetDisplayUsableBounds error: ") + SDL_GetError());
            return;
        }
        position.x = rect.x;
        position.y = rect.y;
        size.x     = rect.w;
        size.y     = rect.h;
    }

    Vec2i getMonitorsSize() const
    {
        Vec2i    size = Vec2i::zero();
        SDL_Rect rect;
        for (int i = 0; i < monitors.size(); i++)
        {
            if (SDL_GetDisplayBounds(monitors[i], &rect) != 0)
            {
                errorAndExit(std::string("SDL_GetDisplayBounds error: ") + SDL_GetError());
                return Vec2i::zero();
            }
            size.x += rect.w;
            size.y += rect.h;
        }
        return size;
    }

    void getMonitorPosition(int index, Vec2i& position) const
    {
        position = Vec2i::zero();
        SDL_Rect rect;
        if (SDL_GetDisplayBounds(monitors[index], &rect) != 0)
        {
            errorAndExit(std::string("SDL_GetDisplayBounds error: ") + SDL_GetError());
            return;
        }
        position.x = rect.x;
        position.y = rect.y;
    }

    void getMonitorSize(int index, Vec2i& size) const
    {
        size = Vec2i::zero();
        SDL_Rect rect;
        if (SDL_GetDisplayBounds(monitors[index], &rect) != 0)
        {
            errorAndExit(std::string("SDL_GetDisplayBounds error: ") + SDL_GetError());
            return;
        }
        size.x += rect.w;
        size.y += rect.h;
    }

    //see: https://github.com/libsdl-org/SDL/blob/main/docs/README-highdpi.md
    float getDisplayContentScale(int index) const
    {
        float rst = SDL_GetDisplayContentScale(monitors[index]);

        if (rst == 0.0f)
        {
            errorAndExit(std::string("SDL_GetDisplayContentScale error: ") + SDL_GetError());
            return 0.0f;
        }
        return rst;
    }

    void addMonitor(SDL_DisplayID monitor)
    {
        monitors.emplace_back(monitor);
    }

    void removeMonitor(const SDL_DisplayID monitor)
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