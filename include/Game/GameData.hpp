#pragma once

#include "Engine/Monitors.hpp"
#include "Engine/Vector2.hpp"

#include <filesystem>
#include <memory>
#include <queue>
#include <vector>
#include <string>

struct GameData
{
    std::unique_ptr<class Window>   window;
    Monitors monitors;
    std::unique_ptr<class InteractionSystem> interactionSystem;

    // Represente the window with all sub windows
    std::vector<std::shared_ptr<class Pet>> pets;
    std::unique_ptr<class ContextualMenu>   contextualMenu;
    std::unique_ptr<class SettingMenu>      settingMenu;

    bool shouldUpdateFrame = true;

    // Resources
    std::unique_ptr<class Framebuffer> pFramebuffer = nullptr;

    std::unique_ptr<class Shader> pImageShader       = nullptr;
    std::unique_ptr<class Shader> pImageGreyScale    = nullptr;
    std::unique_ptr<class Shader> pSpriteSheetShader = nullptr;
    std::vector<std::unique_ptr<class Shader>> edgeDetectionShaders; // Sorted by pass

    std::unique_ptr<class Texture> pCollisionTexture     = nullptr;
    std::unique_ptr<class Texture> pEdgeDetectionTexture = nullptr;

    std::unique_ptr<class ScreenSpaceQuad> pUnitFullScreenQuad = nullptr;
    std::unique_ptr<class ScreenSpaceQuad> pFullScreenQuad     = nullptr;

    Vec2i cursorPos;
    float prevCursorPosX  = 0;
    float prevCursorPosY  = 0;
    float deltaCursorPosX = 0;
    float deltaCursorPosY = 0;
    int   leftButtonEvent = 0;
    int   rightButtonEvent = 0;

    struct DeltaCursosPosElem
    {
        float timer;
        Vec2  pos;

        bool operator>(const DeltaCursosPosElem& other) const noexcept
        {
            return timer > other.timer;
        }
    };

    std::priority_queue<DeltaCursosPosElem, std::vector<DeltaCursosPosElem>, std::greater<DeltaCursosPosElem>>
          deltasCursorPosBuffer;
    float coyoteTimeCursorPos = 0.1f;
    float releaseImpulse      = 3.f;
    Vec2  deltaCursorAcc      = {0.f, 0.f};
    Vec2  pixelPerMeter;

    // Settings
    int FPS        = 0;
    int scale      = 0;
    int randomSeed = 0;

    // Physic
    int  physicFrameRate    = 60;

    // This value is not changed by the physic system. Usefull for movement. Friction is applied to this value
    Vec2  gravity                           = {0.f, 0.f};
    Vec2  gravityDir                        = {0.f, 0.f};
    float bounciness                        = 0.f;
    float friction                          = 0.f;
    float continuousCollisionMaxSqrVelocity = 0.f;
    float collisionPixelRatioStopMovement   = 0.f;
    float isGroundedDetection               = 0.f;
    int   footBasementWidth                 = 1;
    int   footBasementHeight                = 1;

    // Time
    double timeAcc = 0.0;

    // Window
    bool fullScreenWindow          = false;
    bool showWindow                = false;
    bool showFrameBufferBackground = false;
    bool useForwardWindow          = true;
    bool useMousePassThoughWindow  = true;

    // Style
    std::vector<std::filesystem::path> stylesPath;
    std::string styleName;

    // Debug
    bool debugEdgeDetection = false;
};