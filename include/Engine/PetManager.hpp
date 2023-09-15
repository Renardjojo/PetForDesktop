#pragma once

#include "Engine/Singleton.hpp"
#include "Engine/Log.hpp"

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
        std::string filename;
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
        pets.clear();
        for (const auto& entry : std::filesystem::directory_iterator(PETS_PATH))
        {
            std::filesystem::path settingPath = entry.path() / "setting.yaml";
            if (std::filesystem::exists(settingPath))
            {
                const std::string filename = entry.path().filename().string();
                const YAML::Node  node     = YAML::LoadFile(settingPath.string());
                pets.emplace_back(std::make_shared<PetInfo>(PetInfo{filename, node}));
            }
        }
    }

    void createNewPet(const char* filename, const char* name = "", const char* previewPicture = "",
                      const char* author = "")
    {
        YAML::Node        node;
        node["name"]           = name;
        node["previewPicture"] = previewPicture;
        node["author"]         = author;

        std::filesystem::path newDirectoryPath = PETS_PATH;
        newDirectoryPath /= filename;
        std::filesystem::create_directory(newDirectoryPath);
        std::string filePath = (newDirectoryPath / "setting.yaml").string();

        YAML::Emitter out;
        out << node;
     
        FILE* file = nullptr;
        if (fopen_s(&file, filePath.c_str(), "wt"))
        {
            std::filesystem::remove(newDirectoryPath);
            logf("The file \"%s\" was not opened to write\n", filePath);
            return;
        }

        fwrite(out.c_str(), sizeof(char), out.size(), file);
        fclose(file);

        pets.emplace_back(std::make_shared<PetInfo>(PetInfo{filename, node}));
    }

    const std::vector<std::shared_ptr<PetInfo>>& getPetsTypes() const
    {
        return pets;
    }

    std::shared_ptr<PetInfo> getPetType(const char* name) const
    {
        for (auto pet : pets)
        {
            // Use filename to avoid choising the wrong pet if they have the same name
            // if (pet->settings[name].Scalar() == name)
            if (pet->filename == name)
                return pet;
        }

        return nullptr;
    }
};
