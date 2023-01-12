#pragma once

#include "Engine/Vector2.hpp"
#include "Engine/Framebuffer.hpp"
#include "Engine/Shader.hpp"
#include "Engine/ScreenSpaceQuad.hpp"

#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <queue>

struct GameData
{
    // Window and monitor
    GLFWwindow*        window       = nullptr;
    GLFWmonitor**      monitors     = nullptr;
    const GLFWvidmode* videoMode    = nullptr;
    int                monitorCount = 0, monitorX = 0, monitorY = 0;
    Vec2               petPos  = {0.f, 0.f};
    Vec2i              petSize = {0, 0};

    Vec2i windowExt    = {0, 0};
    Vec2i windowMinExt = {0, 0};

    Vec2i windowSize  = {0, 0};
    Vec2i windowPos   = {0, 0};
    Vec2i petPosLimit = {0, 0};

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

    // Inlog
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
    int  physicFrameRate = 60;
    Vec2 velocity        = {0.f, 0.f};
    // This value is not changed by the physic system. Usefull for movement. Friction is applied to this value
    Vec2  continusVelocity                = {0.f, 0.f};
    Vec2  gravity                         = {0.f, 0.f};
    Vec2  gravityDir                      = {0.f, 0.f};
    float bounciness                      = 0.f;
    float friction                        = 0.f;
    float continusCollisionMaxSqrVelocity = 0.f;
    float collisionPixelRatioStopMovement = 0.f;
    float isGroundedDetection             = 0.f;
    int   footBasasementWidth             = 1;
    int   footBasasementHeight            = 1;
    bool  isGrounded                      = false;

    // Animation
    bool side   = true; // false left / true right
    bool isGrab = false;

    // Time
    double timeAcc = 0.0;

    // Window
    bool showWindow                = false;
    bool showFrameBufferBackground = false;
    bool useFowardWindow           = true;
    bool useMousePassThoughWindow  = true;

    // Debug
    bool debugEdgeDetection = false;
};