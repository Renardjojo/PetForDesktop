#pragma once

#include "Engine/Log.hpp"
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

    void createNewPet(const char* filename, const char* name = "", const char* previewPicture = "",
                      const char* author = "")
    {
        YAML::Node node;
        node["name"]           = name;
        node["previewPicture"] = previewPicture;
        node["author"]         = author;

        std::filesystem::path newDirectoryPath = PETS_PATH;
        newDirectoryPath /= filename;
        std::filesystem::create_directory(newDirectoryPath);
        std::string filePath = (newDirectoryPath / "setting.yaml").string();

        if (!saveYAML(node, filePath))
        {
            std::filesystem::remove(newDirectoryPath);
            return;
        }

        pets.emplace_back(
            std::make_shared<PetInfo>(PetInfo{filename, std::filesystem::path(PETS_PATH) / filename, node}));
    }

    void createNewPetAnimation(unsigned int petTypeID, const char* animationName)
    {
        refresh();
        YAML::Node root        = YAML::Node{};
        YAML::Node nodes       = YAML::Node{};
        YAML::Node transitions = YAML::Node{};
        root["FirstNode"]      = "";
        root["PauseNode"]      = "";
        root["Nodes"]          = nodes;
        root["Transitions"]    = transitions;

        std::string filePath = (pets[petTypeID]->rootPath / "animations" / animationName).string();

        if (!saveYAML(root, filePath))
        {
            return;
        }

        pets[petTypeID]->animations.emplace_back(YAMLFile{filePath + ".yaml", root});
    }

    void createAnimation(unsigned int petTypeID, unsigned int animationSetID, const char* animationName)
    {
        refresh();
        YAML::Node animation    = YAML::Node{};
        animation["name"]       = animationName;
        animation["sprite"]     = "";
        animation["sizeFactor"] = 1;
        animation["tileCount"]  = 0;
        animation["framerate"]  = 5;
        animation["loop"]       = true;

        YAML::Node animNode = pets[petTypeID]->animations[animationSetID].file;
        animNode["Nodes"].force_insert("AnimationNode", animation);
        std::string path = pets[petTypeID]->animations[animationSetID].path.string();
        if (!saveYAML(animNode, path))
        {
            return;
        }
    }

    bool saveYAML(YAML::Node& node, std::string& path)
    {
        YAML::Emitter out;
        out << node;

        FILE* file = nullptr;
        if (fopen_s(&file, path.c_str(), "wt"))
        {
            logf("The file \"%s\" was not opened to write\n", path);
            return false;
        }

        fwrite(out.c_str(), sizeof(char), out.size(), file);
        fclose(file);
        return true;
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
