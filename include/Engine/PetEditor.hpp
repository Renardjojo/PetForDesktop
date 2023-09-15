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
    int       m_selectedTransition  = -1;
    int       m_previewCurrentFrame = 0;
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
            displayNodeList();
        }
    }

    void displayNodeList()
    {
        for (YAML::Node& animGraph : datas.animGraphs)
        {
            std::vector<YAML::Node> items;
            ImVec2                  listWinSize = ImGui::GetContentRegionAvail();
            listWinSize.x /= 5;

            displayAnimationList(animGraph, items, listWinSize);

            if (m_selectedNode != -1)
            {
                displayTransitionList(animGraph, items, listWinSize);
                displayAnimationSprite(animGraph, items, listWinSize);
            }
        }
    }

    void displayPetsTypePreview()
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.x /= 2;

        const std::vector<std::shared_ptr<PetManager::PetInfo>>& petTypes = PetManager::instance().getPetsTypes();

        ImGui::BeginChild("PetsTypesWindow", size, true);

        for (int i = 0; i < petTypes.size(); ++i)
        {
            std::string label = petTypes[i]->filename + "##unique_id";

            YAML::Node previewPicture = petTypes[i]->settings["previewPicture"];
            int        previewID      = -1;
            if (previewPicture)
            {
                SpriteSheet* pSprite = datas.spriteSheets.get(previewPicture.Scalar());
                if (pSprite)
                    previewID = pSprite->getID();
            }

            ImVec2 label_size = ImGui::CalcTextSize(label.c_str(), NULL, true);
            ImVec2 selectableSize = label_size;
            selectableSize.y      = 32;

            if (ImGui::Selectable((ImTextureID)previewID, ImVec2(selectableSize.y, selectableSize.y), label.c_str(),
                                  (m_selectedPetType == i), ImGuiSelectableFlags_SpanAvailWidth, selectableSize,
                                  ImVec2(0, 1), ImVec2(1, 0)))
            {
                m_selectedPetType = i;
            }

            YAML::Node authorNode = petTypes[i]->settings["author"];
            if (authorNode)
                ImGui::SetItemTooltip("By %s", authorNode.Scalar().c_str());
        }
        ImGui::EndChild();

        if (m_selectedPetType != -1)
        {
        }
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

    void displayAnimationList(YAML::Node& animGraph, std::vector<YAML::Node>& items, ImVec2 size)
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