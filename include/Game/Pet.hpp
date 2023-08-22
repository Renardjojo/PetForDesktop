#pragma once

#include "Engine/Log.hpp"
#include "Engine/SpriteAnimator.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/StateMachine.hpp"
#include "Engine/UtilitySystem.hpp"
#include "Engine/ClassUtility.hpp"
#include "Game/AnimationTransitions.hpp"
#include "Game/Animations.hpp"
#include "Game/GameData.hpp"
#include "Game/DialoguePopUp.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#endif // USE_OPENGL_API

#include <map>
#include <string>

#include "yaml-cpp/yaml.h"

class Pet : public Rect
{
public:
    enum class ESide
    {
        left  = 0,
        right = 1
    };

protected:

    std::map<std::string, SpriteSheet> spriteSheets;
    ESide                              side{ESide::right};

    GameData& datas;

    StateMachine   animator;
    SpriteAnimator spriteAnimator;
    int            indexCurrentAnimSprite = 0;
    bool           loopCurrentAnim;

    std::string   spritesPath = RESOURCE_PATH "sprites/";
    DialoguePopUp dialoguePopup;
    UtilitySystem utilitySystem;
    NeedUpdator   needUpdator;

    // Animation
    bool isGrab = false;

protected:
    void onChange() final
    {
        Rect::onChange();
        datas.shouldUpdateFrame = true;
    }

public:
    DEFAULT_GETTER_SETTER_VALUE(Side, side)
    DEFAULT_GETTER_SETTER_VALUE(IsGrab, isGrab)

    Pet(GameData& data)
        : datas{data}, animator{data}, dialoguePopup{data}, needUpdator(data, dialoguePopup, utilitySystem)
    {
        data.window->addElement(*this);

        parseAnimationGraph();
        setupUtilitySystem();
    }

    ~Pet()
    {
        datas.window->removeElement(*this);
    }

    void setPosition(const Vec2 position) override
    {
        Rect::setPosition(position);

        if (side == ESide::left)
            dialoguePopup.setPosition(position);
        else
            dialoguePopup.setPosition(position + vec2{m_size.x - dialoguePopup.getSize().x, 0});
    }

    SpriteSheet& getOrAddSpriteSheet(const char* file, int inTileCount, float inSizeFactor)
    {
        auto it = spriteSheets.find(file);
        if (it != spriteSheets.end())
        {
            return it->second;
        }
        else
        {
            return spriteSheets.try_emplace(file, (spritesPath + file).c_str(), inTileCount, inSizeFactor)
                .first->second;
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
            else if (title == "MovementDirectionNode")
            {
                if (!AddMovementNode(it->second, nodes))
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
            else if (title == "TouchScreenEdgeTransition")
            {
                if (!AddBasicTransition<TouchScreenEdgeTransition>(it->second, nodes))
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

    void setupUtilitySystem()
    {
        // Love
        utilitySystem.addNeed(100, 0, 100, 0, 60);
    }

    SpriteSheet& parseAnimation(YAML::Node node)
    {
        YAML::Node sizeFactorNode = node["sizeFactor"];
        YAML::Node tileCountNode  = node["tileCount"];
        return getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str(),
                                   tileCountNode.IsDefined() ? tileCountNode.as<int>() : 1,
                                   sizeFactorNode.IsDefined() ? sizeFactorNode.as<float>() : 1.f);
    }

    template <typename T>
    bool AddBasicNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap())
        {
            warning("YAML error: node invalid");
            return false;
        }
        std::string        nodeName    = node["name"].as<std::string>();
        SpriteSheet&       spriteSheet = parseAnimation(node);
        std::shared_ptr<T> p_node =
            std::make_shared<T>(*this, spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["loop"].as<bool>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddMovementNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap())
        {
            warning("YAML error: MovementDirectionNode invalid");
            return false;
        }
        std::string       nodeName       = node["name"].as<std::string>();
        SpriteSheet&      spriteSheet    = parseAnimation(node);
        int               framerate      = node["framerate"].as<int>();
        YAML::Node        directionsNode = node["directions"];
        std::vector<Vec2> directions;
        bool              applyGravity = node["applyGravity"].as<bool>();
        bool              loop         = node["loop"].as<bool>();

        if (directionsNode.IsSequence())
        {
            for (YAML::const_iterator it = directionsNode.begin(); it != directionsNode.end(); ++it)
            {
                directions.emplace_back(it->as<Vec2>());
            }
        }
        else if (directionsNode.IsScalar())
        {
            directions.emplace_back(directionsNode.as<Vec2>());
        }

        std::shared_ptr<MovementDirectionNode> p_node = std::make_shared<MovementDirectionNode>(
            *this, spriteAnimator, spriteSheet, framerate, directions, applyGravity, loop);
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddJumpNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap())
        {
            warning("YAML error: PetJumpNode invalid");
            return false;
        }
        std::string                  nodeName    = node["name"].as<std::string>();
        SpriteSheet&                 spriteSheet = parseAnimation(node);
        std::shared_ptr<PetJumpNode> p_node      = std::make_shared<PetJumpNode>(
            *this, spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["direction"].as<Vec2>(),
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
        needUpdator.update(deltaTime);
        animator.update(deltaTime);
        dialoguePopup.update(deltaTime);

        // Change screen size
        Vec2  size;
        size.x = spriteAnimator.getSheet()->getWidth() / spriteAnimator.getSheet()->getTileCount() *
                          datas.scale * spriteAnimator.getSheet()->getSizeFactor();
        size.y = spriteAnimator.getSheet()->getHeight() * datas.scale * spriteAnimator.getSheet()->getSizeFactor();
        setSize(size);
    }

    void draw()
    {
        // Drax dialogue pop up
        dialoguePopup.drawIfActive();

        // Draw pet
        spriteAnimator.draw(*this, datas, *datas.pSpriteSheetShader, (bool)side);
        datas.pUnitFullScreenQuad->use();
        datas.pUnitFullScreenQuad->draw();
    }

    bool isMouseOver()
    {  
        const Vec2 localCursorPos           = datas.cursorPos - (m_position - datas.window->getPosition());
        const bool isCursorInsidePetWindow = localCursorPos.x > 0 &&
                                             localCursorPos.y > 0 &&
                                             localCursorPos.x < m_size.x &&
                                             localCursorPos.y < m_size.y;

        if (isCursorInsidePetWindow)
        {
            float scale = datas.scale * spriteAnimator.getSheet()->getSizeFactor();
            Vec2i localCursoPos{
                static_cast<int>(floor(localCursorPos.x / scale)),
                static_cast<int>(floor(localCursorPos.y / scale))};
            return spriteAnimator.isMouseOver(localCursoPos, !(bool)side);
        }
        else
        {
            return false;
        }
    }
};