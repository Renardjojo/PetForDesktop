#pragma once

#include "Engine/Log.hpp"
#include "Engine/Singleton.hpp"
#include "Game/UpdateMenu.hpp"

#include <cpr/cpr.h>
#include <regex>

class Updater : public Singleton<Updater>
{
public:
    void checkForUpdate(GameData& datas)
    {
        cpr::Response response =
            cpr::Get(cpr::Url{"https://api.github.com/repos/Renardjojo/PetDesktop/releases/latest"});
        if (response.error)
        {
            log((response.error.message + "\n").c_str());
            return;
        }

        std::string json = response.text;
        std::regex  pattern("\"tag_name\":\\s*\"(.*?)\"");
        std::smatch matches;

        if (std::regex_search(json, matches, pattern))
        {
            if (matches[1] != "v" PROJECT_VERSION)
            {
                if (!datas.updateMenu)
                {
                    Vec2i mainMonitorPosition;
                    Vec2i mainMonitorSize;
                    datas.monitors.getMainMonitorWorkingArea(mainMonitorPosition, mainMonitorSize);

                    Vec2 menuPosition = mainMonitorPosition + mainMonitorSize / 2;
                    datas.updateMenu  = std::make_unique<UpdateMenu>(datas, menuPosition, json.c_str(), matches[1].str().c_str());
                }
            }
             else
            {
                 logf("The version %s is the latest\n", PROJECT_VERSION);
            }
        }
    }
};