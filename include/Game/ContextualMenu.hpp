#pragma once

#include "Engine/Log.hpp"
#include "Engine/ClassUtility.hpp"

#include "Game/GameData.hpp"
#include "Game/Pet.hpp"

#include "imgui.h"

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
        setSize({50.f * data.scale, 100.f * data.scale});
    }

    ~ContextualMenu()
    {
        datas.window->removeElement(*this);
    }

    void update(double deltaTime)
    {
        ImGui::SetNextWindowPos(ImVec2(m_position.x, m_position.y));
        ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
        ImGui::Begin("Contextual menu");

        ImGui::Text("Contextual menu");

        if (ImGui::Button("Spawn pet"))
        {
            datas.pets.emplace_back(std::make_shared<Pet>(datas));
            shouldClose = true;
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }
};