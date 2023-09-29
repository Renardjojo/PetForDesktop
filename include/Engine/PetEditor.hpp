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
    int                            m_selectedPetType           = -1;
    int                            m_selectedNode              = -1;
    int                            m_selectedAnimation         = -1;
    std::string                    m_selectedAnimationTypeName = "";
    int                            m_selectedTransition        = -1;
    int                            m_previewCurrentFrame       = 0;
    bool                           m_isCreatingPet             = false;
    bool                           m_isCreatingAnimation       = false;
    bool                           m_isCreatingAnimationNode   = false;
    bool                           m_isCreatingTransitionNode  = false;
    char                           m_strBuffer64[64]           = "";
    const std::vector<const char*> m_animationList             = {"AnimationNode", "GrabNode", "MovementDirectionNode",
                                                                  "PetJumpNode"};
    const std::vector<const char*> m_transitionsList           = {
        "StartLeftClicTransition", "TouchScreenEdgeTransition", "IsNotGroundedTransition", "RandomDelayTransition",
        "AnimationEndTransition",  "EndLeftClicTransition",     "IsGroundedTransition"};

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
            if (m_isCreatingPet)
            {
                displayPetCreation();
            }
            else
            {
                displayPetsTypePreview();
            }
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
                displayTransitionList(animGraph.file, items[m_selectedNode], listWinSize);
                // ImGui::SetCursorScreenPos(ImGui::GetCurrentWindow()->DC.CursorPosPrevLine);
                ImGui::SameLine();
                ImGui::GetCurrentWindow()->DC.PrevLineSize = ImVec2(100, 0);
                ImGui::BeginGroup();
                displayAnimationNodeType(animGraph.file, items[m_selectedNode]);
                displayAnimationSprite(animGraph.file, items[m_selectedNode], listWinSize);
                ImGui::EndGroup();
            }
        }
        else
        {
            ImGui::EndGroup();
        }
    }

    void displayAnimationNodeType(YAML::Node& animGraph, YAML::Node& currentAnimationNode)
    {
        if (ImGui::BeginCombo("AnimationType##unique_id", m_selectedAnimationTypeName.c_str()))
        {
            for (int n = 0; n < m_animationList.size(); n++)
            {
                const bool is_selected = (m_selectedAnimationTypeName == m_animationList[n]);
                if (ImGui::Selectable(m_animationList[n], is_selected))
                {
                    m_selectedAnimationTypeName = m_animationList[n];
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
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
                    {
                        m_selectedAnimation  = n;
                        m_selectedNode       = -1;
                        m_selectedTransition = -1;
                    }
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
                        m_selectedNode        = -1;
                        m_selectedTransition  = -1;
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

            addButtonSize.x = ImGui::GetCurrentTable()->BgClipRect.GetWidth();

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

    void displayPetCreation()
    {
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

    void displayAnimationNodeCreation()
    {
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##unique_id", m_strBuffer64, IM_ARRAYSIZE(m_strBuffer64),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (m_strBuffer64[0] != '\0')
            {
                m_isCreatingPet = false;
                // PetManager::instance().createNewPet(m_strBuffer64);
                m_strBuffer64[0] = '\0';
            }
        }
    }

    void displayAnimationSprite(YAML::Node& animGraph, YAML::Node& currentAnimationNode, ImVec2 size)
    {
        // TODO: not safe and don't handle the case if the sprite don't exist in resource manager
        YAML::Node spriteNode = currentAnimationNode["sprite"];
        if (!spriteNode)
            return;

        std::string spriteKey = spriteNode.Scalar();
        if (spriteKey.empty())
            return;

        SpriteSheet* sprite                   = datas.spriteSheets.get(spriteKey);
        int          spriteSheetID            = sprite->getID();
        ImVec2       spriteSheetAvailableSize = size;
        spriteSheetAvailableSize.x =
            size.x * 2 - ImGui::GetStyle().ItemSpacing.x * 2 - ImGui::GetStyle().FramePadding.x * 2;
        spriteSheetAvailableSize.y = std::min(
            sprite->getHeight() / (float)sprite->getWidth() * spriteSheetAvailableSize.x, spriteSheetAvailableSize.y);
        ImGui::ImageWithGrid((ImTextureID)spriteSheetID, spriteSheetAvailableSize,
                             ImVec2(sprite->getWidth(), sprite->getHeight()), ImVec2(0, 1), ImVec2(1, 0),
                             ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5));

        ImVec2 previewSize;

        previewSize.x = spriteSheetAvailableSize.y;
        previewSize.y = spriteSheetAvailableSize.y;

        displayPreview(*sprite, currentAnimationNode, previewSize);
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
        ImVec2 addButtonSize = ImVec2(size.x, 40);
        size.y -= addButtonSize.y + ImGui::GetStyle().FramePadding.y;

        YAML::Node nodesSection = animGraph["Nodes"];
        if (!nodesSection)
            errorAndExit("Cannot find \"Nodes\" in animation.yaml");

        items.reserve(nodesSection.size());

        for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
        {
            items.emplace_back(it->second);
        }

        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

        ImGui::BeginGroup();
        if (ImGui::BeginTable("##unique_id", 1, flags))
        {
            int i = 0;
            for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string label = it->second["name"].Scalar() + "##unique_id";

                if (ImGui::Selectable(label.c_str(), (m_selectedNode == i), ImGuiSelectableFlags_SpanAvailWidth))
                {
                    m_selectedNode              = i;
                    m_selectedAnimationTypeName = it->first.Scalar();
                }
                i++;
            }

            if (m_isCreatingAnimationNode)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##unique_id", m_strBuffer64, IM_ARRAYSIZE(m_strBuffer64),
                                     ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (m_strBuffer64[0] != '\0')
                    {
                        m_isCreatingAnimationNode = false;
                        PetManager::instance().createAnimation(m_selectedPetType, m_selectedAnimation, m_strBuffer64);
                        m_strBuffer64[0] = '\0';
                    }
                }
            }

            addButtonSize.x = ImGui::GetCurrentTable()->BgClipRect.GetWidth();

            ImGui::TableNextRow(0, ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ColumnsMinSpacing -
                                       addButtonSize.y); // Fill the rest of the window with void

            ImGui::EndTable();

            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0, 0.5));
            if (ImGui::Button("Create new animation", addButtonSize))
            {
                m_isCreatingAnimationNode = true;
            }
            ImGui::PopStyleVar();
        }

        ImGui::EndGroup();
    }

    void displayTransitionList(YAML::Node& animGraph, YAML::Node& currentAnimationNode, ImVec2 size)
    {
        ImVec2 addButtonSize = ImVec2(size.x, 40);
        size.y -= addButtonSize.y + ImGui::GetStyle().FramePadding.y;

        YAML::Node  transitionNode   = animGraph["Transitions"];
        std::string nodeSelectedName = currentAnimationNode["name"].Scalar().c_str();

        std::vector<const char*> transitions;

        for (YAML::const_iterator it = transitionNode.begin(); it != transitionNode.end(); ++it)
        {
            auto from = it->second["from"];

            if (from.Scalar() == nodeSelectedName)
                transitions.emplace_back(it->first.Scalar().c_str());
        }

        ImGui::SameLine();

        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                                ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

        ImGui::BeginGroup();
        if (ImGui::BeginTable("TransitionsWindow##unique_id", 1, flags))
        {
            for (int i = 0; i < transitions.size(); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Selectable(transitions[i], m_selectedTransition == i, ImGuiSelectableFlags_SpanAvailWidth))
                {
                    m_selectedTransition = i;
                }
                ImGui::SetItemTooltip("Info transition");
            }

            if (m_isCreatingTransitionNode)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::BeginCombo("##unique_id", ""))
                {
                    for (int n = 0; n < m_transitionsList.size(); n++)
                    {
                        if (ImGui::Selectable(m_transitionsList[n], false))
                        {
                            YAML::Node           nodesSection = animGraph["Nodes"];
                            YAML::const_iterator it           = nodesSection.begin();
                            for (size_t i = 0; i < m_selectedNode; i++)
                            {
                                it++;
                            }

                            PetManager::instance().createTransition(m_selectedPetType, m_selectedAnimation,
                                                                    m_transitionsList[n],
                                                                    it->second["name"].Scalar().c_str(), "");
                            m_isCreatingTransitionNode = false;
                        }
                    }

                    ImGui::EndCombo();
                }
            }

            addButtonSize.x = ImGui::GetCurrentTable()->BgClipRect.GetWidth();

            ImGui::TableNextRow(0, ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ColumnsMinSpacing -
                                       addButtonSize.y); // Fill the rest of the window with void
            ImGui::EndTable();

            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0, 0.5));
            if (ImGui::Button("Create new transition", addButtonSize))
            {
                m_isCreatingTransitionNode = true;
            }
            ImGui::PopStyleVar();
        }
        ImGui::EndGroup();
    }
};