#pragma once

#include "Engine/Singleton.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

class PetManager : public Singleton<PetManager>
{
public:
    struct PetInfo
    {
        std::string name;
        YAML::Node  settings;
    };

protected:
    std::vector<std::shared_ptr<PetInfo>> pets;

public:
    PetManager()
    {
        refresh();
    }

    void refresh()
    {
        for (const auto& entry : std::filesystem::directory_iterator(PETS_PATH))
        {
            std::filesystem::path settingPath = entry.path() / "setting.yaml";
            if (std::filesystem::exists(settingPath))
            {
                const std::string name = entry.path().filename().string();
                const YAML::Node  node = YAML::LoadFile(settingPath.string());
                pets.emplace_back(std::make_shared<PetInfo>(PetInfo{name, node}));
            }
        }
    }

    std::shared_ptr<PetInfo> getPet(const char* name)
    {
        for (auto pet : pets)
        {
            if (pet->name == name)
                return pet;
        }

        return nullptr;
    }
};
