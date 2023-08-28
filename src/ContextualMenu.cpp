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

    ImVec2 sizeCenter = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

    ImGui::BeginDisabled(datas.pets.size() > 2);
    bool shouldSpawnPet = ImGui::Button("Spawn pet", sizeCenter);
    ImGui::EndDisabled();
    if (shouldSpawnPet)
    {
        datas.pets.emplace_back(std::make_shared<Pet>(datas));
        shouldClose = true;
    }

    if (pet.getIsPaused())
    {
        if (ImGui::Button("Resume", sizeCenter))
        {
            pet.setIsPaused(false);
            shouldClose = true;
        }
    }
    else
    {
        if (ImGui::Button("Pause", sizeCenter))
        {
            pet.setIsPaused(true);
            shouldClose = true;
        }
    }

    if (ImGui::Button("Settings", sizeCenter))
    {
        datas.settingMenu = nullptr; // delete previous window
        datas.settingMenu = std::make_unique<SettingMenu>(datas, pet, getPosition());
        datas.window->addElement(*datas.settingMenu);
        shouldClose = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Join us!", sizeCenter))
    {
        SystemOpen("https://discord.gg/gjdQmHAp7e") shouldClose = true;
    }

    if (ImGui::Button("Support this project", sizeCenter))
    {
        SystemOpen("https://www.patreon.com/PetForDesktop") shouldClose = true;
    }

    if (ImGui::Button("Bug report", sizeCenter))
    {
        SystemOpen("https://github.com/Renardjojo/PetForDesktop/issues/new/choose") shouldClose = true;
    }

    if (ImGui::Button("Exit", sizeCenter))
    {
        exit(0);
    }

    std::string txt = std::format("{:.1f} FPS", ImGui::GetIO().Framerate);
    ImGui::SetNextTextLayout(txt.c_str(), 0.5, 0);
    ImGui::Text(txt.c_str());
    windowEnd();
    ImGui::End();
}
