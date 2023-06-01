#pragma once

#include "Game/GameData.hpp"
#include "Game/Pet.hpp"
#include "Engine/Log.hpp"
#include "Engine/PhysicSystem.hpp"
#include "Engine/Settings.hpp"
#include "Engine/SpriteSheet.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/TextureOGL.hpp"
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#include "Engine/Graphics/ShaderOGL.hpp"

#elif USE_DX12_API
#include "Engine/Graphics/TextureDX12.hpp"
#include "Engine/Graphics/ScreenSpaceQuadDX12.hpp"
#include "Engine/Graphics/ShaderDX12.hpp"

#endif // USE_OPENGL_API

#include "Engine/TimeManager.hpp"
#include "Engine/Updater.hpp"
#include "Engine/Utilities.hpp"
#include "Engine/Vector2.hpp"

#include <GLFW/glfw3.h>

#include <functional>

class Game
{
protected:
    Updater      updater;
    GameData     datas;
    Setting      setting;
    TimeManager  mainLoop;
    PhysicSystem physicSystem;

protected:

    void createResources()
    {
#ifdef USE_OPENGL_API
        datas.pFramebuffer = std::make_unique<Framebuffer>();
#endif
        datas.pUnitFullScreenQuad = std::make_unique<ScreenSpaceQuad>(datas.window, 0.f, 1.f);
        datas.pFullScreenQuad     = std::make_unique<ScreenSpaceQuad>(datas.window, -1.f, 1.f);

        datas.edgeDetectionShaders.emplace_back(datas.window, SHADER_RESOURCE_PATH "image" SHADER_VERTEX_EXT,
                                                SHADER_RESOURCE_PATH "dFdxEdgeDetection" SHADER_FRAG_EXT);

        datas.pImageShader = std::make_unique<Shader>(datas.window, SHADER_RESOURCE_PATH "image" SHADER_VERTEX_EXT,
                                                        SHADER_RESOURCE_PATH "image" SHADER_FRAG_EXT);

        if (datas.debugEdgeDetection)
            datas.pImageGreyScale =
                std::make_unique<Shader>(datas.window, SHADER_RESOURCE_PATH "image" SHADER_VERTEX_EXT,
                                                                SHADER_RESOURCE_PATH "imageGreyScale" SHADER_FRAG_EXT);

        datas.pSpriteSheetShader = std::make_unique<Shader>(datas.window, SHADER_RESOURCE_PATH "spriteSheet" SHADER_VERTEX_EXT,
                                                            SHADER_RESOURCE_PATH "image" SHADER_FRAG_EXT);
    }

public:
    Game() : setting(RESOURCE_PATH "setting/setting.yaml", datas), mainLoop(datas), physicSystem(datas)
    {
        logf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);

        glfwSetMonitorCallback(setMonitorCallback);
        datas.window.init(datas);
        datas.monitors.init();
        Vec2i monitorSize    = datas.monitors.getMonitorsSize();
        Vec2i monitorsSizeMM = datas.monitors.getMonitorPhysicalSize();

        // Evaluate pixel distance based on dpi and monitor size
        datas.pixelPerMeter = {(float)monitorSize.x / (monitorsSizeMM.x * 0.001f),
                               (float)monitorSize.y / (monitorsSizeMM.y * 0.001f)};

        createResources();

        srand(datas.randomSeed == -1 ? (unsigned)time(nullptr) : datas.randomSeed);
    }

    ~Game()
    {
        glfwTerminate();
    }

    void runCollisionDetectionMode()
    {
        const std::function<void(double)> unlimitedUpdate{[&](double deltaTime) {
            // poll for and process events
            glfwPollEvents();
        }};

        int                               frameCount = 0;
        const std::function<void(double)> limitedUpdateDebugCollision{[&](double deltaTime) {
            ++frameCount;

            // render
            datas.window.initDrawContext();

            if (!(frameCount & 1) && datas.pImageGreyScale && datas.pEdgeDetectionTexture && datas.pFullScreenQuad)
            {
                datas.pImageGreyScale->use();
                datas.pImageGreyScale->setInt("uTexture", 0);
                datas.pFullScreenQuad->use();
                datas.pEdgeDetectionTexture->use();
                datas.pFullScreenQuad->draw();
            }

            // swap front and back buffers
            datas.window.renderFrame();

            if (frameCount & 1)
            {
                Vec2 newPos;
                physicSystem.CatpureScreenCollision(datas.window.getSize(), newPos);
            }
        }};

        // fullscreen
        Vec2i monitorSize;
        datas.monitors.getMonitorSize(0, monitorSize);
        datas.window.setSize(monitorSize);
        datas.window.setPos(Vec2i::zero());
        glfwSetWindowAttrib(datas.window.getWindow(), GLFW_MOUSE_PASSTHROUGH, true); //TODO: in window
        glfwSetWindowAttrib(datas.window.getWindow(), GLFW_TRANSPARENT_FRAMEBUFFER, true); //TODO: in window
        mainLoop.setFrameRate(1);

        mainLoop.start();
        while (!datas.window.shouldClose())
        {
            mainLoop.update(unlimitedUpdate, limitedUpdateDebugCollision);
        }
    }

    void run()
    {
        if (datas.debugEdgeDetection)
        {
            runCollisionDetectionMode();
            return;
        }

        Pet pet(datas);

        const std::function<void(double)> unlimitedUpdate{[&](double deltaTime) {
            processInput(datas.window.getWindow());

            if (datas.useMousePassThoughWindow)
                glfwSetWindowAttrib(datas.window.getWindow(), GLFW_MOUSE_PASSTHROUGH, !pet.isMouseOver());

            // poll for and process events
            glfwPollEvents();
        }};

        const std::function<void(double)> limitedUpdate{[&](double deltaTime) {
            pet.update(deltaTime);

            if (datas.shouldUpdateFrame)
            {
                datas.window.initDrawContext();

                // render
                pet.draw();

                // swap front and back buffers
                datas.window.renderFrame();
                datas.shouldUpdateFrame = false;
            }
        }};

        Vec2i mainMonitorPosition;
        Vec2i mainMonitorSize;
        datas.monitors.getMainMonitorWorkingArea(mainMonitorPosition, mainMonitorSize);
        datas.window.setPos(mainMonitorPosition + mainMonitorSize / 2);
        datas.petPos    = datas.window.getPos();

#if USE_OPENGL_API
        mainLoop.emplaceTimer([&]() { physicSystem.update(1.f / datas.physicFrameRate); }, 1.f / datas.physicFrameRate,
                              true);
#endif

        mainLoop.start();
        while (!datas.window.shouldClose())
        {
            mainLoop.update(unlimitedUpdate, limitedUpdate);
        }
    }
};