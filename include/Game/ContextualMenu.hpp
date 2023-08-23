#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Log.hpp"

#include "Game/GameData.hpp"
#include "Game/Pet.hpp"

#include "imgui.h"

#include <format>

class ContextualMenu : public Rect
{
protected:
    GameData& datas;
    bool      shouldClose = false;

public:
    GETTER_BY_VALUE(ShouldClose, shouldClose)

    ContextualMenu(GameData& data) : datas{data}
    {
        data.window->addElement(*this);
        setSize({75.f * data.scale, 150.f * data.scale});
    }

    virtual ~ContextualMenu()
    {
        datas.window->removeElement(*this);
    }

    void update(double deltaTime)
    {
        ImGui::SetNextWindowPos(ImVec2(m_position.x, m_position.y));
        ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
        ImGui::Begin("Contextual menu", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoMove);

        ImVec2 sizeCenter = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

        if (ImGui::Button("Spawn pet", sizeCenter))
        {
            datas.pets.emplace_back(std::make_shared<Pet>(datas));
            shouldClose = true;
        }

        if (ImGui::Button("Sleep", sizeCenter))
        {
            shouldClose = true;
        }

        if (ImGui::Button("Settings", sizeCenter))
        {
            shouldClose = true;
        }

        ImGui::Separator();

        if (ImGui::Button("Join us!", sizeCenter))
        {
            shouldClose = true;
        }

        if (ImGui::Button("Support this project", sizeCenter))
        {
            shouldClose = true;
        }

        if (ImGui::Button("Bug report", sizeCenter))
        {
            shouldClose = true;
        }

        textCentered(std::format("{:.1f} FPS", ImGui::GetIO().Framerate));
        ImVec2 contentSize = ImGui::GetWindowSize();
        ImGui::End();
        setSize({contentSize.x, contentSize.y});
    }

    void textCentered(std::string text)
    {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
    }
};
