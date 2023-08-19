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
#endif // USE_OPENGL_API

#include "Engine/TimeManager.hpp"
#include "Engine/Updater.hpp"
#include "Engine/Utilities.hpp"
#include "Engine/Vector2.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
        initUI(datas);

        createResources();

        srand(datas.randomSeed == -1 ? (unsigned)time(nullptr) : datas.randomSeed);
    }

    void initUI(GameData& datas)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(datas.window.getWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    ~Game()
    {
        cleanUI();
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

    void updateUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        //bool show_demo_window = true;
        //ImGui::ShowDemoWindow(&show_demo_window);
    }

    void renderUI()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void cleanUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
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
                updateUI();
                datas.window.initDrawContext();

                // render
                pet.draw();

                renderUI();

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