#pragma once

#include "Game/GameData.hpp"
#include "Game/Pet.hpp"

#include "Engine/Framebuffer.hpp"
#include "Engine/Log.hpp"
#include "Engine/PhysicSystem.hpp"
#include "Engine/ScreenSpaceQuad.hpp"
#include "Engine/Settings.hpp"
#include "Engine/Shader.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/Texture.hpp"
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
        datas.pFramebuffer = std::make_unique<Framebuffer>();
        datas.edgeDetectionShaders.emplace_back(RESOURCE_PATH "shader/image.vs",
                                                RESOURCE_PATH "shader/dFdxEdgeDetection.fs");

        datas.pImageShader = std::make_unique<Shader>(RESOURCE_PATH "shader/image.vs", RESOURCE_PATH "shader/image.fs");

        if (datas.debugEdgeDetection)
            datas.pImageGreyScale =
                std::make_unique<Shader>(RESOURCE_PATH "shader/image.vs", RESOURCE_PATH "shader/imageGreyScale.fs");

        datas.pSpriteSheetShader =
            std::make_unique<Shader>(RESOURCE_PATH "shader/spriteSheet.vs", RESOURCE_PATH "shader/image.fs");

        datas.pUnitFullScreenQuad = std::make_unique<ScreenSpaceQuad>(0.f, 1.f);
        datas.pFullScreenQuad     = std::make_unique<ScreenSpaceQuad>(-1.f, 1.f);

#ifdef _DEBUG
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugMessageCallback, NULL);
#endif
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

    void initDrawContext()
    {
        Framebuffer::bindScreen();
        glViewport(0, 0, datas.window.getSize().x, datas.window.getSize().y);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
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
            initDrawContext();

            if (!(frameCount & 1) && datas.pImageGreyScale && datas.pEdgeDetectionTexture && datas.pFullScreenQuad)
            {
                datas.pImageGreyScale->use();
                datas.pImageGreyScale->setInt("uTexture", 0);
                datas.pFullScreenQuad->use();
                datas.pEdgeDetectionTexture->use();
                datas.pFullScreenQuad->draw();
            }

            // swap front and back buffers
            glfwSwapBuffers(datas.window.getWindow());

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
                // render
                initDrawContext();

                pet.draw();

                // swap front and back buffers
                glfwSwapBuffers(datas.window.getWindow());
                datas.shouldUpdateFrame = false;
            }
        }};

        Vec2i mainMonitorPosition;
        Vec2i mainMonitorSize;
        datas.monitors.getMainMonitorWorkingArea(mainMonitorPosition, mainMonitorSize);
        datas.window.setPos(mainMonitorPosition + mainMonitorSize / 2);
        datas.petPos    = datas.window.getPos();

        mainLoop.emplaceTimer([&]() { physicSystem.update(1.f / datas.physicFrameRate); }, 1.f / datas.physicFrameRate,
                              true);

        mainLoop.start();
        while (!datas.window.shouldClose())
        {
            mainLoop.update(unlimitedUpdate, limitedUpdate);
        }
    }
};