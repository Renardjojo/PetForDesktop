#include "Game/UIMenu.hpp"
#include "Engine/InteractionSystem.hpp"

#include "imgui.h"

UIMenu::UIMenu(GameData& inDatas, Vec2 inPosition, Vec2 inSize) : datas{inDatas}, interactionComponent{*this}
{
    datas.window->addElement(*this);
    datas.interactionSystem->addComponent(interactionComponent);
    m_size     = inSize * datas.scale;
    m_position = inPosition - m_size / 2;
    prevPos    = Vec2(m_position.x - datas.window->getPosition().x, m_position.y - datas.window->getPosition().y);
    onChange();

    shouldInitPosition = true;
}

UIMenu::~UIMenu()
{
    datas.interactionSystem->removeComponent(interactionComponent);
    datas.window->removeElement(*this);
}

void UIMenu::windowBegin()
{
    if (shouldInitPosition)
    {
        ImGui::SetNextWindowPos(
            ImVec2(m_position.x - datas.window->getPosition().x, m_position.y - datas.window->getPosition().y));
        shouldInitPosition = false;
    }

    ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
}

void UIMenu::windowEnd()
{
    ImVec2 contentSize    = ImGui::GetWindowSize();
    ImVec2 contentPos     = ImGui::GetWindowPos();
    Vec2   windowDeltaPos = vec2(contentPos.x - prevPos.x, contentPos.y - prevPos.y);
    prevPos               = Vec2(contentPos.x, contentPos.y);

    // m_position
    setPositionSize({m_position.x + windowDeltaPos.x, m_position.y + windowDeltaPos.y}, {contentSize.x, contentSize.y});
    ImGui::SetWindowPos(
        ImVec2(m_position.x - datas.window->getPosition().x, m_position.y - datas.window->getPosition().y));
    prevPos = Vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
}