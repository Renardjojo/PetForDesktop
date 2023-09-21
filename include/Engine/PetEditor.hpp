#pragma once

#include "Engine/ImGuiTools.hpp"
#include "Engine/PetManager.hpp"
#include "Engine/ResourceManager.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Game/GameData.hpp"

#include "imgui.h"
#include <memory>
#include <vector>

class PetEditor
{
protected:
    int       m_selectedPetType     = -1;
    int       m_selectedNode        = -1;
    int       m_selectedAnimation   = -1;
    int       m_selectedTransition  = -1;
    int       m_previewCurrentFrame = 0;
    bool      m_isCreatingPet       = false;
    bool      m_isCreatingAnimation = false;
    char      m_strBuffer64[64]     = "";
    GameData& datas;

public:
    PetEditor(GameData& data) : datas{data}
    {
    }

    ~PetEditor()
    {
    }

    void execute(float deltaTime)
    {
        if (m_selectedPetType == -1)
        {
            displayPetsTypePreview();
        }
        else
        {
            displayPetTitle();

            if (m_selectedPetType != -1)
            {
                displayNodeList();
            }
        }
    }

    void displayPetTitle()
    {
        const char* petTypeName = PetManager::instance().getPetsTypes()[m_selectedPetType]->filename.c_str();
        ImVec2      prevAlign   = ImGui::GetStyle().SelectableTextAlign;
        ImGui::GetStyle().SelectableTextAlign = ImVec2(0.5, 0.5);
        if (ImGui::Selectable(petTypeName))
        {
            m_selectedPetType = -1;
        }
        ImGui::GetStyle().SelectableTextAlign = prevAlign;
        ImGui::Separator();
    }

    void displayNodeList()
    {
        std::string animPath = RESOURCE_PATH "/setting/animation.yaml";
        YAML::LoadFile(RESOURCE_PATH "/setting/animation.yaml");

        ImVec2 listWinSize = ImGui::GetContentRegionAvail();
        listWinSize.x /= 5;

        ImGui::BeginGroup();

        std::vector<PetManager::YAMLFile>& animations =
            PetManager::instance().getPetsTypes()[m_selectedPetType]->animations;

        displayAnimationList(animations, listWinSize);

        if (m_selectedAnimation != -1)
        {
            PetManager::YAMLFile& animGraph = animations[m_selectedAnimation];

            std::vector<YAML::Node> items;

            displayAnimationNodeList(animGraph.file, items, listWinSize);

            ImGui::EndGroup();

            if (m_selectedNode != -1)
            {
                displayTransitionList(animGraph.file, items, listWinSize);
                displayAnimationSprite(animGraph.file, items, listWinSize);
            }
        }
        else
        {
            ImGui::EndGroup();
        }
    }

    void displayAnimationList(std::vector<PetManager::YAMLFile>& animations, ImVec2 size)
    {
        ImGui::SetNextItemWidth(size.x);

        if (m_isCreatingAnimation)
        {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##unique_id", m_strBuffer64, IM_ARRAYSIZE(m_strBuffer64),
                                 ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (m_strBuffer64[0] != '\0')
                {
                    m_isCreatingAnimation = false;
                    PetManager::instance().createNewPetAnimation(m_selectedPetType, m_strBuffer64);
                    m_strBuffer64[0] = '\0';
                }
            }
        }
        else
        {
            if (ImGui::BeginCombo("##unique_id", m_selectedAnimation == -1
                                                     ? ""
                                                     : animations[m_selectedAnimation].path.stem().string().c_str()))
            {
                for (int n = 0; n < animations.size(); n++)
                {
                    const bool is_selected = (m_selectedAnimation == n);
                    if (ImGui::Selectable(animations[n].path.stem().string().c_str(), is_selected))
                        m_selectedAnimation = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }

                if (m_isCreatingAnimation)
                {
                }
                else
                {
                    if (ImGui::Selectable("Add"))
                    {
                        m_isCreatingAnimation = true;
                    }
                }

                ImGui::EndCombo();
            }
        }
    }

    void displayPetsTypePreview()
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x /= 2;
        ImVec2 addButtonSize = ImVec2(size.x, 40);
        size.y -= addButtonSize.y + ImGui::GetStyle().FramePadding.y;

        const std::vector<std::shared_ptr<PetManager::PetInfo>>& petTypes = PetManager::instance().getPetsTypes();

        ImGui::BeginGroup();
        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

        if (ImGui::BeginTable("##unique_id", 1, flags))
        {
            for (int i = 0; i < petTypes.size(); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string label = petTypes[i]->filename + "##unique_id";

                YAML::Node previewPicture = petTypes[i]->settings["previewPicture"];
                int        previewID      = 0;
                if (previewPicture)
                {
                    SpriteSheet* pSprite = datas.spriteSheets.get(previewPicture.Scalar());
                    if (pSprite)
                        previewID = pSprite->getID();
                }

                ImVec2 label_size     = ImGui::CalcTextSize(label.c_str(), NULL, true);
                ImVec2 selectableSize = label_size;
                selectableSize.y      = 32;

                if (ImGui::Selectable((ImTextureID)previewID, ImVec2(selectableSize.y, selectableSize.y), label.c_str(),
                                      (m_selectedPetType == i), ImGuiSelectableFlags_SpanAvailWidth, selectableSize,
                                      ImVec2(0, 1), ImVec2(1, 0)))
                {
                    m_selectedPetType     = i;
                    m_selectedNode        = -1;
                    m_selectedAnimation   = -1;
                    m_selectedTransition  = -1;
                    m_previewCurrentFrame = 0;
                }

                YAML::Node authorNode = petTypes[i]->settings["author"];
                if (authorNode)
                    ImGui::SetItemTooltip("By %s", authorNode.Scalar().c_str());
            }

            if (m_isCreatingPet)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##unique_id", m_strBuffer64, IM_ARRAYSIZE(m_strBuffer64),
                                     ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (m_strBuffer64[0] != '\0')
                    {
                        m_isCreatingPet = false;
                        PetManager::instance().createNewPet(m_strBuffer64);
                        m_strBuffer64[0] = '\0';
                    }
                }
            }

            addButtonSize.x =
                ImGui::GetColumnWidth() + ImGui::GetStyle().ColumnsMinSpacing + ImGui::GetStyle().CellPadding.x;
            ImGui::TableNextRow(0, ImGui::GetContentRegionAvail().y -
                                       addButtonSize.y); // Fill the rest of the window with void
            ImGui::EndTable();

            if (ImGui::Button("Create new pet", addButtonSize))
            {
                m_isCreatingPet = true;
            }
        }

        ImGui::EndGroup();
    }

    void displayAnimationSprite(YAML::Node& animGraph, std::vector<YAML::Node>& items, ImVec2 size)
    {
        // TODO: not safe and don't handle the case if the sprite don't exist in resource manager
        std::string  spriteKey                = items[m_selectedNode]["sprite"].Scalar();
        SpriteSheet* sprite                   = datas.spriteSheets.get(spriteKey);
        int          spriteSheetID            = sprite->getID();
        ImVec2       spriteSheetAvailableSize = size;
        spriteSheetAvailableSize.x =
            size.x * 2 - ImGui::GetStyle().ItemSpacing.x * 2 - ImGui::GetStyle().FramePadding.x * 2;
        spriteSheetAvailableSize.y = std::min(
            sprite->getHeight() / (float)sprite->getWidth() * spriteSheetAvailableSize.x, spriteSheetAvailableSize.y);
        ImGui::SameLine();
        ImGui::ImageWithGrid((ImTextureID)spriteSheetID, spriteSheetAvailableSize,
                             ImVec2(sprite->getWidth(), sprite->getHeight()), ImVec2(0, 1), ImVec2(1, 0),
                             ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5));

        ImVec2 previewSize;

        previewSize.x = spriteSheetAvailableSize.y;
        previewSize.y = spriteSheetAvailableSize.y;

        displayPreview(*sprite, items[m_selectedNode], previewSize);
    }

    void displayPreview(SpriteSheet& sprite, YAML::Node& currentAnimationNode, ImVec2 size)
    {
        ImGui::SameLine();
        int tileCount         = currentAnimationNode["tileCount"].as<int>();
        int framerate         = currentAnimationNode["framerate"].as<int>();
        m_previewCurrentFrame = std::fmod(framerate * datas.timeAcc, tileCount);

        float uvXStart = m_previewCurrentFrame / (float)tileCount;
        float uvXEnd   = uvXStart + 1 / (float)tileCount;

        ImGui::Image((ImTextureID)sprite.getID(), size, ImVec2(uvXStart, 1), ImVec2(uvXEnd, 0), ImVec4(1, 1, 1, 1),
                     ImVec4(1, 1, 1, 0.5));
    }

    void displayAnimationNodeList(YAML::Node& animGraph, std::vector<YAML::Node>& items, ImVec2 size)
    {
        YAML::Node nodesSection = animGraph["Nodes"];
        if (!nodesSection)
            errorAndExit("Cannot find \"Nodes\" in animation.yaml");

        items.reserve(nodesSection.size());

        for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
        {
            items.emplace_back(it->second);
        }

        ImGui::BeginChild("NodesWindow", size, true);

        for (int i = 0; i < items.size(); ++i)
        {
            if (ImGui::Selectable(items[i]["name"].Scalar().c_str(), m_selectedNode == i))
            {
                m_selectedNode = i;
            }
            ImGui::SetItemTooltip("Info");
        }
        ImGui::EndChild();
    }

    void displayTransitionList(YAML::Node& animGraph, std::vector<YAML::Node>& items, ImVec2 size)
    {
        YAML::Node  transitionNode   = animGraph["Transitions"];
        std::string nodeSelectedName = items[m_selectedNode]["name"].Scalar().c_str();

        std::vector<const char*> transitions;

        for (YAML::const_iterator it = transitionNode.begin(); it != transitionNode.end(); ++it)
        {
            auto from = it->second["from"];

            if (from.Scalar() == nodeSelectedName)
                transitions.emplace_back(it->first.Scalar().c_str());
        }

        ImGui::SameLine();
        ImGui::BeginChild("TransitionsWindow", size, true);
        for (int i = 0; i < transitions.size(); ++i)
        {
            if (ImGui::Selectable(transitions[i], m_selectedTransition == i))
            {
                m_selectedTransition = i;
            }
            ImGui::SetItemTooltip("Info transition");
        }
        ImGui::EndChild();
    }
};