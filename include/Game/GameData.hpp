#pragma once

#include "Engine/Monitors.hpp"
#include "Engine/ScreenSpaceQuad.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Vector2.hpp"

#ifdef USE_OPENGL_API
#include "Engine/WindowOGL.hpp"
#include "Engine/FramebufferOGL.hpp"
#elif USE_DX12_API
#include "Engine/WindowDX12.hpp"
#include "Engine/FramebufferDX12.hpp"
#endif // USE_OPENGL_API

#include <memory>
#include <queue>
#include <vector>

struct GameData
{
    Window   window;
    Monitors monitors;

    Vec2  petPos  = {0.f, 0.f};
    Vec2i petSize = {0, 0};

    Vec2i windowExt    = {0, 0};
    Vec2i windowMinExt = {0, 0};

    bool shouldUpdateFrame = true;

    // Resources
    std::unique_ptr<Framebuffer> pFramebuffer = nullptr;

    std::unique_ptr<Shader> pImageShader       = nullptr;
    std::unique_ptr<Shader> pImageGreyScale    = nullptr;
    std::unique_ptr<Shader> pSpriteSheetShader = nullptr;
    std::vector<Shader>     edgeDetectionShaders; // Sorted by pass

    std::unique_ptr<Texture> pCollisionTexture     = nullptr;
    std::unique_ptr<Texture> pEdgeDetectionTexture = nullptr;

    std::unique_ptr<ScreenSpaceQuad> pUnitFullScreenQuad = nullptr;
    std::unique_ptr<ScreenSpaceQuad> pFullScreenQuad     = nullptr;

    Vec2i cursorPos;
    float prevCursorPosX  = 0;
    float prevCursorPosY  = 0;
    float deltaCursorPosX = 0;
    float deltaCursorPosY = 0;
    int   leftButtonEvent = 0;

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
    Vec2 velocity           = {0.f, 0.f};
    bool applyGravity       = true;
    bool touchScreenEdge    = false;
    bool isOnBottomOfWindow = false;

    // This value is not changed by the physic system. Usefull for movement. Friction is applied to this value
    Vec2  continuousVelocity                = {0.f, 0.f};
    Vec2  gravity                           = {0.f, 0.f};
    Vec2  gravityDir                        = {0.f, 0.f};
    float bounciness                        = 0.f;
    float friction                          = 0.f;
    float continuousCollisionMaxSqrVelocity = 0.f;
    float collisionPixelRatioStopMovement   = 0.f;
    float isGroundedDetection               = 0.f;
    int   footBasementWidth                 = 1;
    int   footBasementHeight                = 1;
    bool  isGrounded                        = false;

    // Animation
    bool side   = true; // false left / true right
    bool isGrab = false;

    // Time
    double timeAcc = 0.0;

    // Window
    bool showWindow                = false;
    bool showFrameBufferBackground = false;
    bool useForwardWindow          = true;
    bool useMousePassThoughWindow  = true;

    // Debug
    bool debugEdgeDetection = false;
};