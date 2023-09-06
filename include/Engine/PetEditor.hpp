#pragma once

#include "imgui.h"
#include "imgui_node_editor.h"

namespace ImGuiNode = ax::NodeEditor;

class PetEditor
{
public:
    // Struct to hold basic information about connection between
    // pins. Note that connection (aka. link) has its own ID.
    // This is useful later with dealing with selections, deletion
    // or other operations.
    struct LinkInfo
    {
        ImGuiNode::LinkId Id;
        ImGuiNode::PinId  InputId;
        ImGuiNode::PinId  OutputId;
    };

    PetEditor()
    {
        ImGuiNode::Config config;
        config.SettingsFile = RESOURCE_PATH "Settings/PetEditorGraph.json";
        m_Context           = ImGuiNode::CreateEditor(&config);
    }

    ~PetEditor()
    {
        ImGuiNode::DestroyEditor(m_Context);
    }

    void ImGuiEx_BeginColumn()
    {
        ImGui::BeginGroup();
    }

    void ImGuiEx_NextColumn()
    {
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
    }

    void ImGuiEx_EndColumn()
    {
        ImGui::EndGroup();
    }

    void Execute(float deltaTime)
    {
        auto& io = ImGui::GetIO();

        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        ImGui::Separator();

        ImGuiNode::SetCurrentEditor(m_Context);

        // Start interaction with editor.
        ImGuiNode::Begin("My Editor", ImVec2(0.0, 0.0f));

        int uniqueId = 1;

        //
        // 1) Commit known data to editor
        //

        // Submit Node A
        ImGuiNode::NodeId nodeA_Id          = uniqueId++;
        ImGuiNode::PinId  nodeA_InputPinId  = uniqueId++;
        ImGuiNode::PinId  nodeA_OutputPinId = uniqueId++;

        if (m_FirstFrame)
            ImGuiNode::SetNodePosition(nodeA_Id, ImVec2(10, 10));
        ImGuiNode::BeginNode(nodeA_Id);
        ImGui::Text("Node A");
        ImGuiNode::BeginPin(nodeA_InputPinId, ImGuiNode::PinKind::Input);
        ImGui::Text("-> In");
        ImGuiNode::EndPin();
        ImGui::SameLine();
        ImGuiNode::BeginPin(nodeA_OutputPinId, ImGuiNode::PinKind::Output);
        ImGui::Text("Out ->");
        ImGuiNode::EndPin();
        ImGuiNode::EndNode();

        // Submit Node B
        ImGuiNode::NodeId nodeB_Id          = uniqueId++;
        ImGuiNode::PinId  nodeB_InputPinId1 = uniqueId++;
        ImGuiNode::PinId  nodeB_InputPinId2 = uniqueId++;
        ImGuiNode::PinId  nodeB_OutputPinId = uniqueId++;

        if (m_FirstFrame)
            ImGuiNode::SetNodePosition(nodeB_Id, ImVec2(210, 60));
        ImGuiNode::BeginNode(nodeB_Id);
        ImGui::Text("Node B");
        ImGuiEx_BeginColumn();
        ImGuiNode::BeginPin(nodeB_InputPinId1, ImGuiNode::PinKind::Input);
        ImGui::Text("-> In1");
        ImGuiNode::EndPin();
        ImGuiNode::BeginPin(nodeB_InputPinId2, ImGuiNode::PinKind::Input);
        ImGui::Text("-> In2");
        ImGuiNode::EndPin();
        ImGuiEx_NextColumn();
        ImGuiNode::BeginPin(nodeB_OutputPinId, ImGuiNode::PinKind::Output);
        ImGui::Text("Out ->");
        ImGuiNode::EndPin();
        ImGuiEx_EndColumn();
        ImGuiNode::EndNode();

        // Submit Links
        for (auto& linkInfo : m_Links)
            ImGuiNode::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

        //
        // 2) Handle interactions
        //

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ImGuiNode::BeginCreate())
        {
            ImGuiNode::PinId inputPinId, outputPinId;
            if (ImGuiNode::QueryNewLink(&inputPinId, &outputPinId))
            {
                // QueryNewLink returns true if editor want to create new link between pins.
                //
                // Link can be created only for two valid pins, it is up to you to
                // validate if connection make sense. Editor is happy to make any.
                //
                // Link always goes from input to output. User may choose to drag
                // link from output pin or input pin. This determine which pin ids
                // are valid and which are not:
                //   * input valid, output invalid - user started to drag new ling from input pin
                //   * input invalid, output valid - user started to drag new ling from output pin
                //   * input valid, output valid   - user dragged link over other pin, can be validated

                if (inputPinId && outputPinId) // both are valid, let's accept link
                {
                    // ImGuiNode::AcceptNewItem() return true when user release mouse button.
                    if (ImGuiNode::AcceptNewItem())
                    {
                        // Since we accepted new link, lets add one to our list of links.
                        m_Links.push_back({ImGuiNode::LinkId(m_NextLinkId++), inputPinId, outputPinId});

                        // Draw new link.
                        ImGuiNode::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }

                    // You may choose to reject connection between these nodes
                    // by calling ImGuiNode::RejectNewItem(). This will allow editor to give
                    // visual feedback by changing link thickness and color.
                }
            }
        }
        ImGuiNode::EndCreate(); // Wraps up object creation action handling.

        // Handle deletion action
        if (ImGuiNode::BeginDelete())
        {
            // There may be many links marked for deletion, let's loop over them.
            ImGuiNode::LinkId deletedLinkId;
            while (ImGuiNode::QueryDeletedLink(&deletedLinkId))
            {
                // If you agree that link can be deleted, accept deletion.
                if (ImGuiNode::AcceptDeletedItem())
                {
                    // Then remove link from your data.
                    for (auto& link : m_Links)
                    {
                        if (link.Id == deletedLinkId)
                        {
                            m_Links.erase(&link);
                            break;
                        }
                    }
                }

                // You may reject link deletion by calling:
                // ImGuiNode::RejectDeletedItem();
            }
        }
        ImGuiNode::EndDelete(); // Wrap up deletion action

        // End of interaction with editor.
        ImGuiNode::End();

        if (m_FirstFrame)
            ImGuiNode::NavigateToContent(0.0f);

        ImGuiNode::SetCurrentEditor(nullptr);

        m_FirstFrame = false;

        // ImGui::ShowMetricsWindow();
    }

    ImGuiNode::EditorContext* m_Context    = nullptr; // Editor context, required to trace a editor state.
    bool               m_FirstFrame = true;    // Flag set for first frame only, some action need to be executed once.
    ImVector<LinkInfo>
        m_Links;            // List of live links. It is dynamic unless you want to create read-only view over nodes.
    int m_NextLinkId = 100; // Counter to help generate link ids. In real application this will probably based on
                            // pointer to user data structure.
};