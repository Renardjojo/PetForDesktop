#include "Game/ContextualMenu.hpp"

#include "Engine/FileExplorer.hpp"
#include "Engine/InteractionSystem.hpp"
#include "Engine/ImGuiTools.hpp"

#include "Game/Pet.hpp"
#include "Game/SettingMenu.hpp"
#include "imgui.h"

#include <cpr/cpr.h>
#include <format>

ContextualMenu::ContextualMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition)
    : UIMenu(inDatas, inPosition, Vec2(75.f, 150.f)), pet{inPet}
{}

ContextualMenu ::~ContextualMenu()
{

}

void ContextualMenu::update(double deltaTime)
{
    windowBegin();

    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);

    bool isWindowOpen = true;
    ImGui::Begin("Contextual menu", &isWindowOpen,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    shouldClose = !isWindowOpen;

    if (datas.leftButtonEvent == GLFW_PRESS && !interactionComponent.isLeftSelected)
        shouldClose = true;

    ImVec2 sizeButton = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

    ImGui::BeginDisabled(datas.pets.size() > 2);
    bool shouldSpawnPet = ImGui::Button("Spawn pet", sizeButton);
    ImGui::EndDisabled();
    if (shouldSpawnPet)
    {
        datas.pets.emplace_back(std::make_shared<Pet>(datas));
        shouldClose = true;
    }

    if (pet.getIsPaused())
    {
        if (ImGui::Button("Resume", sizeButton))
        {
            pet.setIsPaused(false);
            shouldClose = true;
        }
    }
    else
    {
        if (ImGui::Button("Pause", sizeButton))
        {
            pet.setIsPaused(true);
            shouldClose = true;
        }
    }

    if (ImGui::Button("Settings", sizeButton))
    {
        datas.settingMenu = nullptr; // delete previous window
        datas.settingMenu = std::make_unique<SettingMenu>(datas, pet, getPosition());
        datas.window->addElement(*datas.settingMenu);
        shouldClose = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float imageRatio = ImGui::GetTextLineHeight() / datas.pDiscordLogo->getHeight();
    if (ImGui::ImageButtonWithTextRight(
            reinterpret_cast<ImTextureID>(datas.pDiscordLogo->getID()), "Join us!",
            ImVec2(datas.pDiscordLogo->getWidth() * imageRatio, datas.pDiscordLogo->getHeight() * imageRatio),
            sizeButton, ImVec2(ImGui::GetStyle().FramePadding.x * 2, -1)))
    {
        SystemOpen("https://discord.gg/gjdQmHAp7e") shouldClose = true;
    }

    if (ImGui::Button("Support this project", sizeButton))
    {
        SystemOpen("https://www.patreon.com/PetForDesktop") shouldClose = true;
    }

    if (ImGui::Button("Bug report", sizeButton))
    {
        SystemOpen("https://github.com/Renardjojo/PetForDesktop/issues/new/choose") shouldClose = true;
    }

    if (ImGui::Button("Exit", sizeButton))
    {
        exit(0);
    }

    std::string txt = std::format("{:.1f} FPS", ImGui::GetIO().Framerate);
    ImGui::SetNextTextLayout(txt.c_str(), 0.5, 0);
    ImGui::Text(txt.c_str());
    windowEnd();
    ImGui::End();
}
