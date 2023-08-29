#pragma once

#include "Engine/Vector2.hpp"
#include "Game/GameData.hpp"
#include "Game/UIMenu.hpp"

class UpdateMenu : public UIMenu
{
protected:
    std::string content;
    std::string changelog;
    std::vector<std::string> lines;
    std::string windowName;

protected:
    static void changeFileExtension(char* file_name);

    static void generateAndLoadFile(const char* data, size_t count);

        static std::string markdownToPlainText(const std::string& markdown);

public:
    UpdateMenu(GameData& inDatas, Vec2 inPositionn, const char* content, const char* version);

    void update(double deltaTime);
};
