#include "Game/Pet.hpp"

#include "Engine/InteractionSystem.hpp"
#include "Engine/Log.hpp"
#include "Game/AnimationTransitions.hpp"
#include "Game/Animations.hpp"
#include "Game/ContextualMenu.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#endif // USE_OPENGL_API

#include "yaml-cpp/yaml.h"

Pet::Pet(GameData& data)
    : datas{data}, animator{data}, dialoguePopup{data}, needUpdator(data, dialoguePopup, utilitySystem),
      physicComponent(*this), interactionComponent(*this)
{
    data.window->addElement(*this);
    data.interactionSystem->addComponent(interactionComponent);
    interactionComponent.onRightReleaseOver = [&]() { onRightClic(); };

    parseAnimationGraph();
    setupUtilitySystem();
}

Pet::~Pet()
{
    datas.interactionSystem->removeComponent(interactionComponent);
    datas.window->removeElement(*this);
}

void Pet::setIsPaused(bool flag)
{
    if (isPaused == flag)
        return;

    isPaused = flag;
    if (isPaused)
    {
        animator.setCurrent(pauseNode);
        animator.getCurrent()->canUseTransition = false;
    }
    else
    {
        animator.setCurrent(firstNode);
        animator.getCurrent()->canUseTransition = true;
       
    }

    physicComponent.velocity           = {0.f, 0.f};
    physicComponent.continuousVelocity = {0.f, 0.f};
}

void Pet::setPosition(const Vec2 position)
{
    Rect::setPosition(position);

    if (side == ESide::left)
        dialoguePopup.setPosition(position);
    else
        dialoguePopup.setPosition(position + vec2{m_size.x - dialoguePopup.getSize().x, 0});
}

void Pet::setPositionSize(const Vec2 position, const Vec2 size)
{
    Rect::setPositionSize(position, size);

    if (side == ESide::left)
        dialoguePopup.setPosition(position);
    else
        dialoguePopup.setPosition(position + vec2{m_size.x - dialoguePopup.getSize().x, 0});
}

SpriteSheet& Pet::getOrAddSpriteSheet(const char* file, int inTileCount, float inSizeFactor)
{
    auto it = spriteSheets.find(file);
    if (it != spriteSheets.end())
    {
        return it->second;
    }
    else
    {
        return spriteSheets.try_emplace(file, (spritesPath + file).c_str(), inTileCount, inSizeFactor).first->second;
    }
}

void Pet::parseAnimationGraph()
{
    YAML::Node animGraph = YAML::LoadFile(RESOURCE_PATH "/setting/animation.yaml");

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

    // Optional Pause node
    YAML::Node pauseNodeSetting = animGraph["PauseNode"];
    if (pauseNodeSetting)
    {
        std::string pauseNodeName = pauseNodeSetting.as<std::string>();
        auto                               it = nodes.find(pauseNodeName);
        if (it != nodes.end())
        {
            pauseNode = it->second;
        }
    }

    // First node
    bool        firstNodeFound   = false;
    YAML::Node firstNodeSetting = animGraph["FirstNode"];

    if (firstNodeSetting)
    {
        std::string firstNodeName = firstNodeSetting.as<std::string>();

        // Start state machine
        firstNode = nodes[firstNodeName];

        if (firstNode != nullptr)
        {
            animator.init(std::static_pointer_cast<StateMachine::Node>(firstNode));
            firstNodeFound = true;
        }
    }

    if (!firstNodeFound)
    {
        animator.init(std::static_pointer_cast<StateMachine::Node>(nodes.begin()->second));
        warning("FirstNode name is invalid. First node selected instead");
    }
}

void Pet::setupUtilitySystem()
{
    // Love
    utilitySystem.addNeed(100, 0, 100, 0, 60);
}

SpriteSheet& Pet::parseAnimation(YAML::Node node)
{
    YAML::Node sizeFactorNode = node["sizeFactor"];
    YAML::Node tileCountNode  = node["tileCount"];
    return getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str(),
                               tileCountNode.IsDefined() ? tileCountNode.as<int>() : 1,
                               sizeFactorNode.IsDefined() ? sizeFactorNode.as<float>() : 1.f);
}

template <typename T>
bool Pet::AddBasicNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
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

bool Pet::AddMovementNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
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

bool Pet::AddJumpNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
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
bool Pet::AddBasicTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
{
    if (!node.IsMap() || node.size() != 2)
    {
        warning("YAML error: transition invalid");
        return false;
    }

    std::string nodeFromName = node["from"].as<std::string>();
    YAML::Node  toNodes      = node["to"];

    std::shared_ptr<T> transition = std::make_shared<T>(*this);
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

bool Pet::AddRandomDelayTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
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

void Pet::update(double deltaTime)
{
    if (isPaused)
        return;

    needUpdator.update(deltaTime);
    dialoguePopup.update(deltaTime);

    if (interactionComponent.isLeftRelease)
        physicComponent.velocity =
            datas.deltaCursorAcc / datas.coyoteTimeCursorPos / datas.pixelPerMeter * datas.releaseImpulse;

    if (interactionComponent.isLeftPressOver)
        physicComponent.isGrounded = false;
}

void Pet::updateRendering(double deltaTime)
{
    animator.update(deltaTime);

    // Change screen size
    Vec2 size;
    size.x = spriteAnimator.getSheet()->getWidth() / spriteAnimator.getSheet()->getTileCount() * datas.scale *
             spriteAnimator.getSheet()->getSizeFactor();
    size.y = spriteAnimator.getSheet()->getHeight() * datas.scale * spriteAnimator.getSheet()->getSizeFactor();
    setSize(size);
}

void Pet::draw()
{
    // Drax dialogue pop up
    dialoguePopup.drawIfActive();

    // Draw pet
    spriteAnimator.draw(*this, datas, *datas.pSpriteSheetShader, (bool)side);
    datas.pUnitFullScreenQuad->use();
    datas.pUnitFullScreenQuad->draw();
}

bool Pet::isPointInside(Vec2 pointPos)
{
    if (Rect::isPointInside(pointPos))
    {
        float scale = datas.scale * spriteAnimator.getSheet()->getSizeFactor();
        Vec2i localPointPos{static_cast<int>(floor(pointPos.x / scale)), static_cast<int>(floor(pointPos.y / scale))};
        return spriteAnimator.isMouseOver(localPointPos, !(bool)side);
    }
    else
    {
        return false;
    }
}

void Pet::onRightClic()
{
    datas.contextualMenu = nullptr; // delete previous window
    datas.contextualMenu = std::make_unique<ContextualMenu>(datas, *this, getPosition());
    datas.window->addElement(*datas.contextualMenu);
}
