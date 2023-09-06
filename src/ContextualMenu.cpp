#include "Game/ContextualMenu.hpp"

#include "Engine/Localization.hpp"
#include "Engine/FileExplorer.hpp"
#include "Engine/ImGuiTools.hpp"
#include "Engine/InteractionSystem.hpp"

#include "Game/Pet.hpp"
#include "Game/SettingMenu.hpp"
#include "imgui.h"

#include <cpr/cpr.h>
#include <format>

ContextualMenu::ContextualMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition)
    : UIMenu(inDatas, inPosition, Vec2(100.f, 150.f)), pet{inPet}
{
}

ContextualMenu ::~ContextualMenu()
{
}

void ContextualMenu::update(double deltaTime)
{
    windowBegin();

    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);

    bool isWindowOpen = true;
    ImGui::Begin(Localization::instance().getLocal("ContextualMenu", "Contextual menu").c_str(), &isWindowOpen,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    shouldClose = !isWindowOpen;

    if (datas.leftButtonEvent == GLFW_PRESS && !interactionComponent.isLeftSelected)
        shouldClose = true;

    ImVec2 sizeButton = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

    ImGui::BeginDisabled(ImGui::GetIO().Framerate < 30);
    bool shouldSpawnPet =
        ImGui::Button(Localization::instance().getLocal("SpawnPet", "Spawn pet").c_str(), sizeButton);
    ImGui::EndDisabled();
    if (shouldSpawnPet)
    {
        Vec2i mainMonitorPosition;
        Vec2i mainMonitorSize;
        datas.monitors.getMainMonitorWorkingArea(mainMonitorPosition, mainMonitorSize);

        for (size_t i = 0; i < 250; i++)
        {
            Vec2 petPosition = mainMonitorPosition;
            petPosition.x += randNum(0, mainMonitorSize.x);
            petPosition.y += randNum(0, mainMonitorSize.y);
            datas.pets.emplace_back(std::make_shared<Pet>(datas, petPosition));
        }
        shouldClose = true;
    }

    if (pet.getIsPaused())
    {
        if (ImGui::Button(Localization::instance().getLocal("Resume").c_str(), sizeButton))
        {
            pet.setIsPaused(false);
            shouldClose = true;
        }
    }
    else
    {
        if (ImGui::Button(Localization::instance().getLocal("Pause").c_str(), sizeButton))
        {
            pet.setIsPaused(true);
            shouldClose = true;
        }
    }

    if (ImGui::Button(Localization::instance().getLocal("Settings").c_str(), sizeButton))
    {
        datas.settingMenu = nullptr; // delete previous window
        datas.settingMenu = std::make_unique<SettingMenu>(datas, pet, getPosition());
        datas.window->addElement(*datas.settingMenu);
        shouldClose = true;
    }

    // Next content at the end of the window
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y -
                         ImGui::GetTextLineHeightWithSpacing() * 5 - ImGui::GetStyle().FramePadding.y * 5 - 1);
    ImGui::Separator();

    float  imageRatio = ImGui::GetTextLineHeight() / datas.pDiscordLogo->getHeight();
    ImVec2 imageSize =
        ImVec2(datas.pDiscordLogo->getWidth() * imageRatio, datas.pDiscordLogo->getHeight() * imageRatio);
    ImVec2 imageTextPadding = ImVec2(ImGui::GetStyle().FramePadding.x * 2, -1);

    if (ImGui::ImageButtonWithTextRight(reinterpret_cast<ImTextureID>(datas.pDiscordLogo->getID()),
                                        Localization::instance().getLocal("JoinUs", "Join us!").c_str(),
                                        imageSize, sizeButton, imageTextPadding))
    {
        SystemOpen("https://discord.gg/gjdQmHAp7e") shouldClose = true;
    }

    imageRatio = ImGui::GetTextLineHeight() / datas.pPatreonLogo->getHeight();
    imageSize  = ImVec2(datas.pPatreonLogo->getWidth() * imageRatio, datas.pPatreonLogo->getHeight() * imageRatio);
    if (ImGui::ImageButtonWithTextRight(reinterpret_cast<ImTextureID>(datas.pPatreonLogo->getID()),
            Localization::instance().getLocal("Support", "Support this project").c_str(), imageSize,
            sizeButton, imageTextPadding))
    {
        SystemOpen("https://www.patreon.com/PetForDesktop") shouldClose = true;
    }

    if (ImGui::Button(Localization::instance().getLocal("BugReport", "Bug report").c_str(),
                      sizeButton))
    {
        SystemOpen("https://github.com/Renardjojo/PetForDesktop/issues/new/choose") shouldClose = true;
    }


    if (ImGui::Button(Localization::instance().getLocal("Exit").c_str(), sizeButton))
    {
        exit(0);
    }

    std::string txt = std::format("{:.1f} FPS", ImGui::GetIO().Framerate);
    ImGui::SetNextTextLayout(txt.c_str(), 0.5, 0);
    ImGui::Text(txt.c_str());
    windowEnd();
    ImGui::End();
}
