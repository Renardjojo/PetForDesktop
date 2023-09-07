#pragma once

#include "Game/GameData.hpp"
#include "imgui.h"

class PetEditor
{
protected:
    int       m_selectedNode = -1;
    int       m_selectedTransition = -1;
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
            YAML::Node nodesSection = animGraph["Nodes"];
            if (!nodesSection)
                errorAndExit("Cannot find \"Nodes\" in animation.yaml");

            std::vector<const char*> items;
            items.reserve(nodesSection.size());

            for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
            {
                items.emplace_back(it->second["name"].Scalar().c_str());
            }

            ImVec2 listWinSize = ImGui::GetContentRegionAvail();
            listWinSize.x /= 5;
            ImGui::BeginChild("NodesWindow", listWinSize, true);

            for (int i = 0; i < items.size(); ++i)
            {
                if (ImGui::Selectable(items[i], m_selectedNode == i))
                {
                    m_selectedNode = i;
                }
                ImGui::SetItemTooltip("Info");
            }
            ImGui::EndChild();

            if (m_selectedNode != -1)
            {
                YAML::Node  transitionNode = animGraph["Transitions"];
                std::string nodeSelectedName    = items[m_selectedNode];

                std::vector<const char*> transitions;

                for (YAML::const_iterator it = transitionNode.begin(); it != transitionNode.end(); ++it)
                {
                    auto from = it->second["from"];

                    if (from.Scalar() == nodeSelectedName)
                        transitions.emplace_back(it->first.Scalar().c_str());
                }

                ImGui::SameLine();
                ImGui::BeginChild("TransitionsWindow", listWinSize, true);
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

        }
    }
};