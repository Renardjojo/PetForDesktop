#pragma once

#include "Engine/Singleton.hpp"
#include "Engine/Log.hpp"

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
        YAML::Node  file;
    };

    struct PetInfo
    {
        std::string             filename;
        YAML::Node              settings;
        std::vector<YAMLFile>   animations;
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

                pet->filename   = entry.path().filename().string();
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
