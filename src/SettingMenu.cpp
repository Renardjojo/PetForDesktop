#include "Game/SettingMenu.hpp"

#include "Engine/InteractionSystem.hpp"
#include "Engine/StylePanel.hpp"

#include "Game/Pet.hpp"
#include "imgui.h"

SettingMenu::SettingMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition)
    : datas{inDatas}, pet{inPet}, interactionComponent{*this}
{
    datas.window->addElement(*this);
    datas.interactionSystem->addComponent(interactionComponent);
    m_size     = {400.f * datas.scale, 400.f * datas.scale};
    m_position = inPosition;
    onChange();

    shouldInitPosition = true;
}

SettingMenu::~SettingMenu()
{
    datas.interactionSystem->removeComponent(interactionComponent);
    datas.window->removeElement(*this);
}

void SettingMenu::update(double deltaTime)
{
    if (shouldInitPosition)
    {
        ImGui::SetNextWindowPos(
            ImVec2(m_position.x - datas.window->getPosition().x, m_position.y - datas.window->getPosition().y));
        shouldInitPosition = false;
    }

    //ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));

    bool isWindowOpen = true;
    ImGui::Begin("Settings", &isWindowOpen,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    shouldClose = !isWindowOpen;

    if (datas.leftButtonEvent == GLFW_PRESS && !interactionComponent.isLeftSelected)
        shouldClose = true;

    ShowStyleEditor();

    ImVec2 contentSize = ImGui::GetWindowSize();
    ImVec2 contentPos = ImGui::GetWindowPos();
    ImGui::End();

    setPositionSize({datas.window->getPosition().x + contentPos.x, datas.window->getPosition().y + contentPos.y},
                    {contentSize.x, contentSize.y});
}