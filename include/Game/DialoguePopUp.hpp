#pragma once

#include "Engine/Log.hpp"
#include "Engine/UtilitySystem.hpp"
#include "Game/GameData.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#endif // USE_OPENGL_API

#include <map>
#include <string>

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

class DialoguePopUp : public Rect
{
protected:
    GameData&                     datas;
    std::string                   emotesPath = RESOURCE_PATH "/sprites/emote/";
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

        data.window->addElement(*this);

        m_size = {25.f * data.scale, 25.f * data.scale};
    }

    ~DialoguePopUp()
    {
        datas.window->removeElement(*this);
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

            Vec2 clipSpacePos = Vec2::remap(getCornerMin(), datas.window->getCornerMin(),
                                            datas.window->getCornerMax(), Vec2{0, 1}, Vec2{1, 0}); // [-1, 1]
            Vec2 clipSpaceSize =
                Vec2::remap(getSize(), Vec2{0, 0}, datas.window->getSize(), Vec2{0, 0}, Vec2{1, 1}); // [0, 1]

            // In shader, based on bottom left instead of upper left
            clipSpacePos.y -= clipSpaceSize.y;

            datas.pSpriteSheetShader->setVec4("uScaleOffSet", 1, 1, 0, 0);
            datas.pSpriteSheetShader->setVec4("uClipSpacePosSize", clipSpacePos.x, clipSpacePos.y, clipSpaceSize.x,
                                              clipSpaceSize.y);

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
        nexTimeDisplayPopup = randNum(1000, 3000) / 1000.f;
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