#pragma once

#include "Engine/InteractionSystem.hpp"
#include "Engine/Log.hpp"
#include "Engine/PhysicSystem.hpp"
#include "Engine/Settings.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/StylePanel.hpp"
#include "Game/ContextualMenu.hpp"
#include "Game/SettingMenu.hpp"
#include "Game/UpdateMenu.hpp"
#include "Game/GameData.hpp"
#include "Game/Pet.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#include "Engine/Graphics/ShaderOGL.hpp"
#include "Engine/Graphics/TextureOGL.hpp"
#endif // USE_OPENGL_API

#include "Engine/TimeManager.hpp"
#include "Engine/Updater.hpp"
#include "Engine/Utilities.hpp"
#include "Engine/Vector2.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include <GLFW/glfw3.h>

#include <functional>

class Game
{
protected:
    GameData     datas;
    PhysicSystem physicSystem;

protected:
    void createResources()
    {
        datas.pFramebuffer = std::make_unique<Framebuffer>();

        datas.pUnitFullScreenQuad = std::make_unique<ScreenSpaceQuad>(*datas.window, 0.f, 1.f);
        datas.pFullScreenQuad     = std::make_unique<ScreenSpaceQuad>(*datas.window, -1.f, 1.f);

        datas.edgeDetectionShaders.emplace_back(
            std::make_unique<Shader>(*datas.window, SHADER_RESOURCE_PATH "/image" SHADER_VERTEX_EXT,
                                     SHADER_RESOURCE_PATH "/dFdxEdgeDetection" SHADER_FRAG_EXT));

        datas.pImageShader = std::make_unique<Shader>(*datas.window, SHADER_RESOURCE_PATH "/image" SHADER_VERTEX_EXT,
                                                      SHADER_RESOURCE_PATH "/image" SHADER_FRAG_EXT);

        if (datas.debugEdgeDetection)
            datas.pImageGreyScale =
                std::make_unique<Shader>(*datas.window, SHADER_RESOURCE_PATH "/image" SHADER_VERTEX_EXT,
                                         SHADER_RESOURCE_PATH "/imageGreyScale" SHADER_FRAG_EXT);

        datas.pSpriteSheetShader =
            std::make_unique<Shader>(*datas.window, SHADER_RESOURCE_PATH "/spriteSheet" SHADER_VERTEX_EXT,
                                     SHADER_RESOURCE_PATH "/image" SHADER_FRAG_EXT);


        datas.pDiscordLogo =
            std::make_unique<Texture>(RESOURCE_PATH "/sprites/logo/discord-mark-blue.png", false, Texture::linearClampSampling);
        datas.pPatreonLogo = std::make_unique<Texture>(RESOURCE_PATH "/sprites/logo/Digital-Patreon-Logo_FieryCoral.png", false,
                                                       Texture::linearClampSampling);
    }

public:
    Game() : physicSystem(datas)
    {
        logf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);
        
        Setting::instance().importFile(RESOURCE_PATH "/setting/setting.yaml", datas);
        TimeManager::instance().Init(datas);

        glfwSetMonitorCallback(setMonitorCallback);
        datas.window = std::make_unique<Window>();
        datas.window->init(datas);
        datas.monitors.init();
        Vec2i monitorSize    = datas.monitors.getMonitorsSize();
        Vec2i monitorsSizeMM = datas.monitors.getMonitorPhysicalSize();

        if (datas.fullScreenWindow)
        {
            datas.window->setSize(monitorSize);
        }
        else
        {
            datas.window->setPosition(monitorSize / 2);
        }

        // Evaluate pixel distance based on dpi and monitor size
        datas.pixelPerMeter = {(float)monitorSize.x / (monitorsSizeMM.x * 0.001f),
                               (float)monitorSize.y / (monitorsSizeMM.y * 0.001f)};

        datas.interactionSystem = std::make_unique<InteractionSystem>();

        initUI(datas);

        createResources();

        srand(datas.randomSeed == -1 ? (unsigned)time(nullptr) : datas.randomSeed);

#if NDEBUG // Check for update only on release to avoid harassing the server
        Updater::instance().checkForUpdate(datas);
#endif
        datas.pets.emplace_back(std::make_shared<Pet>(datas));
    }

    void initUI(GameData& datas)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // Load style
        bool useDefaultProfile = true;
        setDefaultTheme(); // fallback theme
        io.Fonts->AddFontFromFileTTF(RESOURCE_PATH "/fonts/NimbusSanL-Reg.otf", 14 * datas.textScale);

        // Get all styles
        for (const auto& entry : fs::directory_iterator(RESOURCE_PATH "/styles"))
        {
            std::filesystem::path path = entry.path();
            if (path.extension() == ".style")
            {
                datas.stylesPath.emplace_back(path);

                if (path.stem() == datas.styleName)
                {
                    ImGuiLoadStyle(path.string().c_str(), ImGui::GetStyle());
                    useDefaultProfile = false;
                }
            }
        }

        if (useDefaultProfile)
            datas.styleName = "default";

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(datas.window->getWindow(), true);
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
            datas.window->initDrawContext();

            if (!(frameCount & 1) && datas.pImageGreyScale && datas.pEdgeDetectionTexture && datas.pFullScreenQuad)
            {
                datas.pImageGreyScale->use();
                datas.pImageGreyScale->setInt("uTexture", 0);
                datas.pFullScreenQuad->use();
                datas.pEdgeDetectionTexture->use();
                datas.pFullScreenQuad->draw();
            }

            // swap front and back buffers
            datas.window->renderFrame();

            if (frameCount & 1)
            {
                Vec2 newPos;
                for (const std::shared_ptr<Pet>& pet : datas.pets)
                {
                    physicSystem.CatpureScreenCollision(*pet, datas.window->getSize(), newPos);
                }
            }
        }};

        // fullscreen
        Vec2i monitorSize;
        datas.monitors.getMonitorSize(0, monitorSize);
        datas.window->setSize(monitorSize);
        datas.window->setPosition(Vec2::zero());
        TimeManager::instance().setFrameRate(1);

        TimeManager::instance().start();
        while (!datas.window->shouldClose())
        {
            TimeManager::instance().update(unlimitedUpdate, limitedUpdateDebugCollision);
        }
    }

    void updateUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
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

        const std::function<void(double)> unlimitedUpdate{[&](double deltaTime) {
            processInput(datas.window->getWindow());

            // poll for and process events
            glfwPollEvents();

            datas.interactionSystem->update(datas);

            for (const std::shared_ptr<Pet>& pet : datas.pets)
            {
                pet->update(deltaTime);
            }
        }};

        const std::function<void(double)> limitedUpdate{[&](double deltaTime) {
            for (const std::shared_ptr<Pet>& pet : datas.pets)
            {
                pet->updateRendering(deltaTime);
            }

            if (datas.shouldUpdateFrame)
            {
                updateUI();

                // TODO: make generic class to avoid code repetition
                if (datas.contextualMenu != nullptr)
                {
                    datas.contextualMenu->update(deltaTime);

                    if (datas.contextualMenu->getShouldClose())
                        datas.contextualMenu = nullptr;
                }

                if (datas.settingMenu != nullptr)
                {
                    datas.settingMenu->update(deltaTime);

                    if (datas.settingMenu->getShouldClose())
                        datas.settingMenu = nullptr;
                }

                if (datas.updateMenu != nullptr)
                {
                    datas.updateMenu->update(deltaTime);

                    if (datas.updateMenu->getShouldClose())
                        datas.updateMenu = nullptr;
                }

                datas.window->initDrawContext();

                // render
                for (const std::shared_ptr<Pet>& pet : datas.pets)
                {
                    pet->draw();
                }

                renderUI();

                // swap front and back buffers
                datas.window->renderFrame();
                datas.shouldUpdateFrame = false;
            }
        }};

        Vec2i mainMonitorPosition;
        Vec2i mainMonitorSize;
        datas.monitors.getMainMonitorWorkingArea(mainMonitorPosition, mainMonitorSize);
        for (size_t i = 0; i < datas.pets.size(); i++)
        {
            Vec2 petPosition = mainMonitorPosition;
            petPosition.y += mainMonitorSize.y / 2.f;
            petPosition.x += mainMonitorSize.x / (datas.pets.size() + 1) * (i + 1);
            datas.pets[i]->setPosition(petPosition);
        }

        TimeManager::instance().emplaceTimer(
            [&]() {
                for (const std::shared_ptr<Pet>& pet : datas.pets)
                {
                    physicSystem.update(pet->getPhysicComponent(), pet->getInteractionComponent(),
                                        1.f / datas.physicFrameRate);
                }
            },
            1.f / datas.physicFrameRate, true);

        TimeManager::instance().start();
        while (!datas.window->shouldClose())
        {
            TimeManager::instance().update(unlimitedUpdate, limitedUpdate);
        }
    }
};