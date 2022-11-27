#pragma once

#include "Engine/StateMachine.hpp"
#include "Engine/SpriteAnimator.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/Log.hpp"
#include "Game/GameData.hpp"
#include "Game/Animations.hpp"
#include "Game/AnimationTransitions.hpp"

#include <map>
#include <string>

#include "yaml-cpp/yaml.h"

class Pet
{
protected:
    enum class ESide
    {
        left,
        right
    };

    std::map<std::string, SpriteSheet> spriteSheets;
    ESide                              side{ESide::left};

    GameData& datas;

    StateMachine   animator;
    SpriteAnimator spriteAnimator;
    int            indexCurrentAnimSprite = 0;
    bool           loopCurrentAnim;

    bool        leftWasPressed = false;
    std::string spritesPath    = RESOURCE_PATH "sprites/";

public:
    Pet(GameData& data) : datas{data}, animator{data}
    {
        parseAnimationGraph();
    }

    SpriteSheet& getOrAddSpriteSheet(const char* file)
    {
        auto it = spriteSheets.find(file);
        if (it != spriteSheets.end())
        {
            return it->second;
        }
        else
        {
            return spriteSheets.emplace(file, (spritesPath + file).c_str()).first->second;
        }
    }

    void parseAnimationGraph()
    {
        YAML::Node animGraph = YAML::LoadFile(RESOURCE_PATH "setting/animation.yaml");

        // Init nodes
        std::map<std::string, std::shared_ptr<StateMachine::Node>> nodes;

        YAML::Node nodesSection = animGraph["Nodes"];
        if (!nodesSection)
            errorAndExit("Cannot find \"Nodes\" in animation.yaml");

        for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
        {
            std::string title = it->first.Scalar();
            // TODO hash
            if (title == "AnimationNode")
            {
                if (!AddBasicNode<AnimationNode>(it->second, nodes))
                    continue;
            }
            else if (title == "GrabNode")
            {
                if (!AddBasicNode<GrabNode>(it->second, nodes))
                    continue;
            }
            else if (title == "PetWalkNode")
            {
                if (!AddWalkNode(it->second, nodes))
                    continue;
            }
            else if (title == "PetJumpNode")
            {
                if (!AddJumpNode(it->second, nodes))
                    continue;
            }
            else
            {
                warning(std::string("Node with name ") + title + " isn't implemented and is skiped");
            }
        }

        // Init transitions
        YAML::Node transitionsSection = animGraph["Transitions"];
        if (!transitionsSection)
            errorAndExit("Cannot find \"Transitions\" in animation.yaml");

        for (YAML::const_iterator it = transitionsSection.begin(); it != transitionsSection.end(); ++it)
        {
            std::string title = it->first.Scalar();
            // TODO hash
            if (title == "StartLeftClicTransition")
            {
                if (!AddBasicTransition<StartLeftClicTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "IsNotGroundedTransition")
            {
                if (!AddBasicTransition<IsNotGroundedTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "RandomDelayTransition")
            {
                if (!AddRandomDelayTransition(it->second, nodes))
                    continue;
            }
            else if (title == "AnimationEndTransition")
            {
                if (!AddBasicTransition<AnimationEndTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "EndLeftClicTransition")
            {
                if (!AddBasicTransition<EndLeftClicTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "IsGroundedTransition")
            {
                if (!AddBasicTransition<IsGroundedTransition>(it->second, nodes))
                    continue;
            }
            else
            {
                warning(std::string("Transition with name ") + title + " isn't implemented and is skiped");
            }
        }

        // First node
        YAML::Node firstNode = animGraph["FirstNode"];
        if (!firstNode)
            errorAndExit("Cannot find \"FirstNode\" in animation.yaml");

        // Start state machine
        std::string                          firstNodeName = firstNode.as<std::string>();
        std::shared_ptr<StateMachine::Node>& firstSMNode   = nodes[firstNodeName];

        if (firstSMNode != nullptr)
        {
            animator.init(std::static_pointer_cast<StateMachine::Node>(firstSMNode));
        }
        else
        {
            animator.init(std::static_pointer_cast<StateMachine::Node>(nodes.begin()->second));
            warning("FirstNode name is invalid. First node selected instead");
        }
    }

    template <typename T>
    bool AddBasicNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 4)
        {
            warning("YAML error: node invalid");
            return false;
        }
        std::string        nodeName    = node["name"].as<std::string>();
        SpriteSheet&       spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<T> p_node =
            std::make_shared<T>(spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["loop"].as<bool>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddWalkNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 6)
        {
            warning("YAML error: PetWalkNode invalid");
            return false;
        }
        std::string                  nodeName    = node["name"].as<std::string>();
        SpriteSheet&                 spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<PetWalkNode> p_node      = std::make_shared<PetWalkNode>(
            spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["direction"].as<Vec2>(),
            node["thrust"].as<float>(), node["loop"].as<bool>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddJumpNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 6)
        {
            warning("YAML error: PetJumpNode invalid");
            return false;
        }
        std::string                  nodeName    = node["name"].as<std::string>();
        SpriteSheet&                 spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<PetJumpNode> p_node      = std::make_shared<PetJumpNode>(
            spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["direction"].as<Vec2>(),
            node["verticalThrust"].as<float>(), node["horizontalThrust"].as<float>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    template <typename T>
    bool AddBasicTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 2)
        {
            warning("YAML error: transition invalid");
            return false;
        }

        std::string nodeFromName = node["from"].as<std::string>();
        YAML::Node  toNodes      = node["to"];

        std::shared_ptr<T> transition = std::make_shared<T>();
        if (toNodes.IsSequence())
        {
            for (YAML::const_iterator it = toNodes.begin(); it != toNodes.end(); ++it)
            {
                transition->to.emplace_back(nodes[it->as<std::string>()]);
            }
        }
        else if (toNodes.IsScalar())
        {
            transition->to.emplace_back(nodes[toNodes.as<std::string>()]);
        }

        nodes[nodeFromName]->AddTransition(std::static_pointer_cast<StateMachine::Node::Transition>(transition));
        return true;
    }

    bool AddRandomDelayTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 4)
        {
            warning("YAML error: transition invalid");
            return false;
        }
        std::string nodeFromName = node["from"].as<std::string>();
        YAML::Node  toNodes      = node["to"];
        int         duration     = node["duration"].as<int>();
        int         interval     = node["interval"].as<int>();

        std::shared_ptr<RandomDelayTransition> transition = std::make_shared<RandomDelayTransition>(duration, interval);

        if (toNodes.IsSequence())
        {
            for (YAML::const_iterator it = toNodes.begin(); it != toNodes.end(); ++it)
            {
                transition->to.emplace_back(nodes[it->as<std::string>()]);
            }
        }
        else if (toNodes.IsScalar())
        {
            transition->to.emplace_back(nodes[toNodes.as<std::string>()]);
        }
        nodes[nodeFromName]->AddTransition(std::static_pointer_cast<StateMachine::Node::Transition>(transition));
        return true;
    }

    void update(double deltaTime)
    {
        animator.update(deltaTime);
    }

    void draw()
    {
        spriteAnimator.draw(datas, *datas.pSpriteSheetShader, datas.side);
        datas.pUnitFullScreenQuad->use();
        datas.pUnitFullScreenQuad->draw();
    }
};