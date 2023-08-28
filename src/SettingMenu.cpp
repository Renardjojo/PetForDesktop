#include "Game/SettingMenu.hpp"

#include "Engine/FileExplorer.hpp"
#include "Engine/InteractionSystem.hpp"
#include "Engine/StylePanel.hpp"
#include "Engine/Settings.hpp"

#include "Game/Pet.hpp"
#include "imgui.h"

SettingMenu::SettingMenu(GameData& inDatas, Pet& inPet, Vec2 inPosition)
    : UIMenu(inDatas, inPosition, Vec2(400.f, 400.f)), pet{inPet}
{}

SettingMenu::~SettingMenu()
{
    Setting::instance().exportFile(RESOURCE_PATH "/setting/setting.yaml", datas);
}

void SettingMenu::update(double deltaTime)
{
    windowBegin();

    bool isWindowOpen = true;
    ImGui::Begin("Settings", &isWindowOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    shouldClose = !isWindowOpen;

    if (datas.leftButtonEvent == GLFW_PRESS && !interactionComponent.isLeftSelected)
        shouldClose = true;

    if (ImGui::BeginTabBar("##settingMenuTabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Game"))
        {
            if (ImGui::BeginCombo("Style", datas.styleName.c_str()))
            {
                for (int i = 0; i < datas.stylesPath.size(); i++)
                {
                    auto& path = datas.stylesPath[i];
                    auto  fileName = path.stem();
                    if (ImGui::Selectable(fileName.string().c_str(), fileName == datas.styleName))
                    {
                        ImGuiLoadStyle(path.string().c_str(), ImGui::GetStyle());
                        datas.styleName = fileName.string();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Content"))
        {
            if (ImGui::Button("Open content folder"))
                SystemOpen(RESOURCE_PATH);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Style"))
        {
            ShowStyleEditor();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Accessibility"))
        {
            ImGui::DragInt("Global scale", &datas.scale, 0.05f, 1, 10);
            if (ImGui::DragFloat("Font scale", &datas.textScale, 0.005f, 0.3f, 2.0f, "%.1f"))
            {
                ImGui::GetFont()->Scale = datas.textScale;
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    windowEnd();
    ImGui::End();
}