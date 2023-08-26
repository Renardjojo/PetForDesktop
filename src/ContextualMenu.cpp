#include "Game/ContextualMenu.hpp"

#include "Engine/InteractionSystem.hpp"
#include "Engine/FileExplorer.hpp"

#include "Game/Pet.hpp"
#include "Game/SettingMenu.hpp"
#include "imgui.h"

#include <cpr/cpr.h>
#include <format>

ContextualMenu::ContextualMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition)
    : datas{inDatas}, pet{inPet}, interactionComponent{*this}
{
    datas.window->addElement(*this);
    datas.interactionSystem->addComponent(interactionComponent);
    m_size     = {75.f * datas.scale, 150.f * datas.scale};
    m_position = inPosition;
    onChange();

    shouldInitPosition = true;
}

ContextualMenu ::~ContextualMenu()
{
    datas.interactionSystem->removeComponent(interactionComponent);
    datas.window->removeElement(*this);
}

void ContextualMenu::update(double deltaTime)
{
    if (shouldInitPosition)
    {
        ImGui::SetNextWindowPos(
            ImVec2(m_position.x - datas.window->getPosition().x, m_position.y - datas.window->getPosition().y));
        shouldInitPosition = false;
    }

    ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);

    bool isWindowOpen = true;
    ImGui::Begin("Contextual menu", &isWindowOpen,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    shouldClose = !isWindowOpen;

    if (datas.leftButtonEvent == GLFW_PRESS && !interactionComponent.isLeftSelected)
        shouldClose = true;

    ImVec2 sizeCenter = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

    if (ImGui::Button("Spawn pet", sizeCenter))
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
            shouldClose  = true;
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
        SystemOpen("https://discord.gg/gjdQmHAp7e")
        shouldClose = true;
    }

    if (ImGui::Button("Support this project", sizeCenter))
    {
        SystemOpen("https://www.patreon.com/PetForDesktop")
        shouldClose = true;
    }

    if (ImGui::Button("Bug report", sizeCenter))
    {
        SystemOpen("https://github.com/Renardjojo/PetForDesktop/issues/new/choose")
        shouldClose = true;
    }

    textCentered(std::format("{:.1f} FPS", ImGui::GetIO().Framerate));
    ImVec2 contentSize = ImGui::GetWindowSize();
    ImVec2 contentPos = ImGui::GetWindowPos();
    ImGui::End();

    setPositionSize({datas.window->getPosition().x + contentPos.x, datas.window->getPosition().y + contentPos.y},
                    {contentSize.x, contentSize.y});
}

void ContextualMenu::textCentered(std::string text)
{
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
}
