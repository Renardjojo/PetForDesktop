#pragma once

#include "Engine/ResourceManager.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Game/GameData.hpp"
#include "imgui.h"

class PetEditor
{
protected:
    int       m_selectedNode       = -1;
    int       m_selectedTransition = -1;
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
        displayNodeList();
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
        ImGui::Image((ImTextureID)spriteSheetID, spriteSheetAvailableSize, ImVec2(0, 1), ImVec2(1, 0),
                     ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5));

        ImVec2 previewSize;

        previewSize.x = spriteSheetAvailableSize.y;
        previewSize.y = spriteSheetAvailableSize.y;

        displayPreview(*sprite, items[m_selectedNode], previewSize);
    }

    void displayPreview(SpriteSheet& sprite, YAML::Node& currentAnimationNode, ImVec2 size)
    {
        ImGui::SameLine();
        int         tileCount = currentAnimationNode["tileCount"].as<int>();
        int         framerate = currentAnimationNode["framerate"].as<int>();
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