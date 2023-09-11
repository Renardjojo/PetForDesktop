#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/PhysicComponent.hpp"
#include "Engine/SpriteAnimator.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/StateMachine.hpp"
#include "Engine/UtilitySystem.hpp"
#include "Game/DialoguePopUp.hpp"
#include "Game/GameData.hpp"

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
    ESide                              side{ESide::right};

    GameData& datas;

    StateMachine   animator;
    SpriteAnimator spriteAnimator;
    int            indexCurrentAnimSprite = 0;
    bool           loopCurrentAnim;

    std::string   spritesPath = RESOURCE_PATH "/sprites/";
    DialoguePopUp dialoguePopup;
    UtilitySystem utilitySystem;
    NeedUpdator   needUpdator;

    // Animation
    std::shared_ptr<StateMachine::Node> firstNode;
    std::shared_ptr<StateMachine::Node> pauseNode;

    bool isGrab = false;
    bool isPaused = false;

    // Components
    PhysicComponent      physicComponent;
    InteractionComponent interactionComponent;

protected:
    void onChange() final
    {
        Rect::onChange();
        datas.shouldUpdateFrame = true;
    }

public:
    DEFAULT_GETTER_SETTER_VALUE(Side, side)
    DEFAULT_GETTER_SETTER_VALUE(IsGrab, isGrab)

    GETTER_BY_VALUE(IsPaused, isPaused)
    GETTER_BY_REF(PhysicComponent, physicComponent)
    GETTER_BY_REF(InteractionComponent, interactionComponent)

    Pet(GameData& data, Vec2 position);

    ~Pet();

    void setIsPaused(bool flag);

    void setPosition(const Vec2 position) override;

    void setPositionSize(const Vec2 position, const Vec2 size) override;

    SpriteSheet& getOrAddSpriteSheet(const char* file, int inTileCount, float inSizeFactor);

    void parseAnimationGraph();

    void setupUtilitySystem();

    SpriteSheet& parseAnimation(YAML::Node node);

    template <typename T>
    bool AddBasicNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes);

    bool AddMovementNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes);

    bool AddJumpNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes);

    template <typename T>
    bool AddBasicTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes);

    bool AddRandomDelayTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes);

    void update(double deltaTime);

    void updateRendering(double deltaTime);

    void draw();

    virtual bool isPointInside(Vec2 pointPos);

    void onRightClic();
};