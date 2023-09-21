#pragma once

#include "Engine/Singleton.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

#include <iostream>

class PetManager : public Singleton<PetManager>
{
public:
    struct YAMLFile
    {
        std::filesystem::path path;
        YAML::Node            file;
    };

    struct PetInfo
    {
        std::string           filename;
        std::filesystem::path rootPath;
        YAML::Node            settings;
        std::vector<YAMLFile> animations;
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
        pets.clear();
        for (const auto& entry : std::filesystem::directory_iterator(PETS_PATH))
        {
            std::filesystem::path path = entry.path() / "setting.yaml";
            if (std::filesystem::exists(path))
            {
                std::shared_ptr<PetInfo> pet = std::make_shared<PetInfo>();

                pet->filename = entry.path().filename().string();
                pet->rootPath = entry.path();
                pet->settings = YAML::LoadFile(path.string());

                path = entry.path() / "animations";
                if (std::filesystem::exists(path))
                {
                    for (const auto& entry : std::filesystem::directory_iterator(path))
                    {
                        if (entry.is_regular_file())
                        {
                            pet->animations.emplace_back(YAMLFile{entry.path(), YAML::LoadFile(entry.path().string())});
                        }
                    }
                }

                pets.emplace_back(std::move(pet));
            }
        }
    }

    void createNewPet(const char* name)
    {
        refresh();
        const std::string filename = name;
        YAML::Node        node     = YAML::Node{};
        pets.emplace_back(std::make_shared<PetInfo>(PetInfo{filename, std::filesystem::path(PETS_PATH) / filename, node}));
    }

    void createNewPetAnimation(unsigned int petTypeID, const char* animationName)
    {
        refresh();
        YAML::Node node = YAML::Node{};
        pets[petTypeID]->animations.emplace_back(
            YAMLFile{pets[petTypeID]->rootPath / "animations" / animationName, node});
    }

    const std::vector<std::shared_ptr<PetInfo>>& getPetsTypes() const
    {
        return pets;
    }

    std::shared_ptr<PetInfo> getPetType(const char* name) const
    {
        for (auto& pet : pets)
        {
            // Use filename to avoid choising the wrong pet if they have the same name
            // if (pet->settings[name].Scalar() == name)
            if (pet->filename == name)
                return pet;
        }

        return nullptr;
    }
};
