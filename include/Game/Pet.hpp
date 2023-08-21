#pragma once

#include "Engine/Log.hpp"
#include "Engine/SpriteAnimator.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/StateMachine.hpp"
#include "Engine/UtilitySystem.hpp"
#include "Game/AnimationTransitions.hpp"
#include "Game/Animations.hpp"
#include "Game/GameData.hpp"

#include <map>
#include <string>

#include "yaml-cpp/yaml.h"

enum class EPopupType
{
    Dialogue = 0,
    Dream,
    Exclamation,

    COUNT
};

enum class ENeed
{
    Love = 0,
    Sleep,
    Hungry,
    Sad,
    Happy,
    Angry,
    None,
    COUNT
};

class DialoguePopUp
{
protected:
    GameData&                     datas;
    std::string                   emotesPath = RESOURCE_PATH "sprites/emote/";
    std::map<EPopupType, Texture> popups;
    std::map<ENeed, Texture>      speachs;

    bool       m_isActive              = false;
    float      m_displayDuration       = 0.f;
    float      m_displayerCurrentTimer = 0.f;
    EPopupType m_backgroundToDisplay;
    ENeed      m_forgroundToDisplay;

public:
    DialoguePopUp(GameData& data) : datas{data}
    {
        popups.emplace(EPopupType::Dialogue, (emotesPath + "emote1_.png").c_str());

        speachs.emplace(ENeed::Love, (emotesPath + "heart.png").c_str());
        speachs.emplace(ENeed::Sleep, (emotesPath + "sleep2.png").c_str());
        speachs.emplace(ENeed::Hungry, (emotesPath + "drop1.png").c_str());
        speachs.emplace(ENeed::Sad, (emotesPath + "faceSad.png").c_str());
        speachs.emplace(ENeed::Happy, (emotesPath + "faceHappy.png").c_str());
        speachs.emplace(ENeed::Angry, (emotesPath + "faceAngry.png").c_str());
    }

    void update(double deltaTime)
    {
        if (m_isActive)
        {
            m_displayerCurrentTimer += deltaTime;

            if (m_displayerCurrentTimer >= m_displayDuration)
            {
                m_isActive = false;
            }
        }
    }

    void display(float displayDuration, EPopupType backgroundToDisplay, ENeed forgroundToDisplay)
    {
        m_isActive              = true;
        m_displayerCurrentTimer = 0.f;
        m_displayDuration       = displayDuration;
        m_backgroundToDisplay   = backgroundToDisplay;
        m_forgroundToDisplay    = forgroundToDisplay;
    }

    void drawIfActive()
    {
        if (m_isActive)
        {
            datas.pSpriteSheetShader->use();
            datas.pSpriteSheetShader->setVec4("uScaleOffSet", 1, 1, 0, 0);
            datas.pSpriteSheetShader->setVec4("uClipSpacePosSize", 0.33, 0.5, 0.33, 0.33);

            datas.pUnitFullScreenQuad->use();

            popups.at(m_backgroundToDisplay).use();
            datas.pUnitFullScreenQuad->draw();

            speachs.at(m_forgroundToDisplay).use();
            datas.pUnitFullScreenQuad->draw();
        }
    }
};

class NeedUpdator
{
    DialoguePopUp& m_dialoguePopup;
    UtilitySystem& m_utilitySystem;
    GameData&      m_datas;
    int            lastNeed;
    bool           leftWasPressed           = false;
    float          displayPopupCurrentTimer = 0.f;
    float          nexTimeDisplayPopup      = 0.f;

public:
    NeedUpdator(GameData& datas, DialoguePopUp& dialoguePopup, UtilitySystem& utilitySystem)
        : m_datas{datas}, m_dialoguePopup{dialoguePopup}, m_utilitySystem{utilitySystem}
    {
        nexTimeDisplayPopup = randNum(10000, 30000) / 1000.f;
    }

    void update(float deltaTime)
    {
        int currentNeedIndex = m_utilitySystem.getPriority();

        if (currentNeedIndex != -1)
        {
            if (currentNeedIndex != lastNeed)
            {
                m_dialoguePopup.display(1.f, EPopupType::Dialogue, ENeed::Angry);
                displayPopupCurrentTimer = 0;
                lastNeed                 = currentNeedIndex;
                nexTimeDisplayPopup      = randNum(10000, 30000) / 1000.f;
            }
            else
            {
                displayPopupCurrentTimer += deltaTime;

                if (displayPopupCurrentTimer > nexTimeDisplayPopup)
                {
                    displayPopupCurrentTimer -= nexTimeDisplayPopup;
                    nexTimeDisplayPopup = randNum(10000, 30000) / 1000.f;
                    m_dialoguePopup.display(2.f, EPopupType::Dialogue, ENeed::Angry);
                }
            }
        }
        else
        {
            displayPopupCurrentTimer += deltaTime;

            if (displayPopupCurrentTimer > nexTimeDisplayPopup)
            {
                displayPopupCurrentTimer -= nexTimeDisplayPopup;
                nexTimeDisplayPopup = randNum(10000, 30000) / 1000.f;
                m_dialoguePopup.display(2.f, EPopupType::Dialogue, ENeed::Love);
            }
        }

        for (size_t i = 0; i < m_utilitySystem.needs.size(); i++)
        {
            m_utilitySystem.needs[i].reduce(0.5 * deltaTime);
        }

        if (m_datas.leftButtonEvent == GLFW_PRESS)
        {
            leftWasPressed = true;
        }
        else if (leftWasPressed)
        {
            leftWasPressed = false;
            m_utilitySystem.needs[(int)ENeed::Love].add(100);
        }
    }
};

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

    std::string   spritesPath = RESOURCE_PATH "sprites/";
    DialoguePopUp dialoguePopup;
    UtilitySystem utilitySystem;
    NeedUpdator   needUpdator;

public:
    Pet(GameData& data)
        : datas{data}, animator{data}, dialoguePopup{data}, needUpdator(data, dialoguePopup, utilitySystem)
    {
        data.petRect = std::make_shared<Rect>();
        data.window.addElement(*data.petRect);

        parseAnimationGraph();
        setupUtilitySystem();
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
            std::make_shared<T>(spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["loop"].as<bool>());
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
            spriteAnimator, spriteSheet, framerate, directions, applyGravity, loop);
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
        needUpdator.update(deltaTime);
        animator.update(deltaTime);
        dialoguePopup.update(deltaTime);

        // Change screen size
        Vec2  size;
        size.x = spriteAnimator.getSheet()->getWidth() / spriteAnimator.getSheet()->getTileCount() *
                          datas.scale * spriteAnimator.getSheet()->getSizeFactor();
        size.y = spriteAnimator.getSheet()->getHeight() * datas.scale * spriteAnimator.getSheet()->getSizeFactor();
        datas.petRect->setSize(size);
        Vec2i windowSize{(int)std::floor(datas.petRect->getSize().x), (int)std::floor(datas.petRect->getSize().y)};
        datas.window.setSize(windowSize);
    }

    void draw()
    {
        // Drax dialogue pop up
        dialoguePopup.drawIfActive();

        // Draw pet
        spriteAnimator.draw(datas, *datas.pSpriteSheetShader, datas.side);
        datas.pUnitFullScreenQuad->use();
        datas.pUnitFullScreenQuad->draw();
    }

    bool isMouseOver()
    {
        const Vec2 localWinPos             = datas.petRect->getPosition() - datas.window.getPosition();
        const bool isCursorInsidePetWindow = datas.cursorPos.x > localWinPos.x && datas.cursorPos.y > localWinPos.y &&
                                             datas.cursorPos.x < localWinPos.x + (float)datas.petRect->getSize().x &&
                                             datas.cursorPos.y < localWinPos.y + (float)datas.petRect->getSize().y;

        if (isCursorInsidePetWindow)
        {
            Vec2i localCursoPos{
                static_cast<int>(
                    floor(datas.cursorPos.x / (float)(datas.scale * spriteAnimator.getSheet()->getSizeFactor()))),
                static_cast<int>(
                    floor(datas.cursorPos.y / (float)(datas.scale * spriteAnimator.getSheet()->getSizeFactor())))};
            return spriteAnimator.isMouseOver(localCursoPos, !datas.side);
        }
        else
        {
            return false;
        }
    }
};