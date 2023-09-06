#include "Game/UpdateMenu.hpp"

#include "Engine/Localization.hpp"
#include "Engine/FileExplorer.hpp"
#include "Engine/ImGuiTools.hpp"
#include "Engine/Log.hpp"
#include "imgui.h"

#include <cpr/cpr.h>
#include <regex>
#include <string>

void UpdateMenu::changeFileExtension(char* file_name)
{
    // Find the last '.' character in the file name
    char* dot = strrchr(file_name, '.');
    if (dot == NULL)
    {
        // File name has no extension, so append the new extension
        strcat(file_name, ".exe");
    }
    else
    {
        // File name has an extension, so copy the file name up to the '.' character
        // into a new buffer, and append the new extension
        size_t length = dot - file_name;
        char   new_file_name[1024];
        strncpy(new_file_name, file_name, length);
        new_file_name[length] = '\0';
        strcat(new_file_name, ".exe");
        strncpy(file_name, new_file_name, length + 5);
    }
}

void UpdateMenu::generateAndLoadFile(const char* data, size_t count)
{
    // Generate a unique file name
    char temp_file_name[1024] = {0};
    if (tmpnam(temp_file_name) == NULL)
        return;

    // Create the temporary file
    changeFileExtension(temp_file_name);
    FILE* temp_file = fopen(temp_file_name, "w+b");
    if (temp_file == NULL)
        return;

    fwrite(data, sizeof(char), count, temp_file);
    fclose(temp_file);

    SystemOpen(temp_file_name)

        unlink(temp_file_name);
    exit(0);
}

std::string UpdateMenu::markdownToPlainText(const std::string& markdown)
{
    // Replace '\\r' and '\\n' with '\n'
    std::string lineBreakSearch  = "\\\\n|\\\\r";
    std::string lineBreakReplace = "\n";
    std::regex  lineBreakRegex(lineBreakSearch);
    std::string plainText = std::regex_replace(markdown, lineBreakRegex, lineBreakReplace);

    // Regular expressions for Markdown syntax
    std::regex headerRegex("#+\\s*(.*)");
    std::regex listRegex("\\*\\s+(.*)");
    std::regex linkRegex("\\[(.*?)\\]\\((https?://\\S+)\\)");
    std::regex boldRegex("\\*\\*(.*?)\\*\\*");

    // Replace headers with plain text
    plainText = std::regex_replace(plainText, headerRegex, "$1");

    // Replace list items with plain text
    plainText = std::regex_replace(plainText, listRegex, "- $1");

    // Replace links with plain text
    plainText = std::regex_replace(plainText, linkRegex, "$2");

    // Remove bold sections
    plainText = std::regex_replace(plainText, boldRegex, "$1");

    return plainText;
}

UpdateMenu::UpdateMenu(GameData& inDatas, Vec2 inPosition, const char* inContent, const char* version)
    : UIMenu(inDatas, inPosition, Vec2(400.f, 200.f)), content{inContent}
{
    std::regex  pattern(R"("body"\s*:\s*"([^"]*)\")");
    std::smatch matches;
    windowName = std::string(PROJECT_NAME " ") + version + " is available.";

    // Looking for changelog
    if (std::regex_search(content, matches, pattern))
    {
        changelog = markdownToPlainText(matches[1].str());
    }
}

void UpdateMenu::update(double deltaTime)
{
    windowBegin();

    bool isWindowOpen = true;

    ImGui::Begin(windowName.c_str(), &isWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    shouldClose = !isWindowOpen;

    ImVec2 sizeButton = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

    std::istringstream       iss(changelog);
    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(iss, line))
    {
        lines.push_back(line);
    }

    // Process each line and log the desired parts using ImGui::Text
    for (const std::string& line : lines)
    {
        size_t urlStartPos = line.find("https://github.com/");
        if (urlStartPos != std::string::npos)
        {
            std::string authorText = line.substr(0, urlStartPos);
            std::string urlText    = line.substr(urlStartPos);

            // Log the author text and URL using ImGui::Text
            ImGui::Text("%s", authorText.c_str());
            ImGui::textURL(urlText.c_str(), urlText.c_str(), 1, 0);
        }
        else
        {
            // Log the entire line as plain text using ImGui::Text
            ImGui::Text("%s", line.c_str());
        }
    }

    if (ImGui::Button(Localization::instance().getLocal("Update").c_str(),
                      sizeButton))
    {
        std::regex  pattern("\"browser_download_url\":\\s*\"(.*?)\"");
        std::smatch matches;
        if (std::regex_search(content, matches, pattern))
        {
            cpr::Response response = cpr::Get(cpr::Url{matches[1]});
            generateAndLoadFile(response.text.c_str(), response.text.size());
        }
        else
        {
            log("Update package not found\n");
        }
    }

    windowEnd();
    ImGui::End();
}
