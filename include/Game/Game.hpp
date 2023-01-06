#pragma once

#include "Game/Pet.hpp"
#include "Game/GameData.hpp"

#include "Engine/PhysicSystem.hpp"
#include "Engine/Utilities.hpp"
#include "Engine/Settings.hpp"
#include "Engine/SpriteSheet.hpp"
#include "Engine/Window.hpp"
#include "Engine/ScreenSpaceQuad.hpp"
#include "Engine/TimeManager.hpp"
#include "Engine/Framebuffer.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Log.hpp"
#include "Engine/Vector2.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cpr/cpr.h>

#include <functional>
#include <regex>

class Game
{
protected:
    GameData     datas;
    Setting      setting;
    TimeManager  mainLoop;
    PhysicSystem physicSystem;

protected:
    void initWindow()
    {
        // initialize the library
        if (!glfwInit())
            errorAndExit("glfw initialization error");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, !datas.showFrameBufferBackground);
        glfwWindowHint(GLFW_VISIBLE, datas.showFrameBufferBackground);
        glfwWindowHint(GLFW_FLOATING, datas.useFowardWindow);

        // Disable depth and stencil buffers
        glfwWindowHint(GLFW_DEPTH_BITS, 0);
        glfwWindowHint(GLFW_STENCIL_BITS, 0);

        datas.monitors    = glfwGetMonitors(&datas.monitorCount);
        datas.videoMode   = glfwGetVideoMode(datas.monitors[0]);
        datas.petPosLimit = {datas.videoMode->width, datas.videoMode->height};
        datas.windowSize  = {1, 1};

        datas.window = glfwCreateWindow(datas.windowSize.x, datas.windowSize.y, PROJECT_NAME, NULL, NULL);
        if (!datas.window)
        {
            glfwTerminate();
            errorAndExit("Create Window error");
        }

        glfwMakeContextCurrent(datas.window);

        glfwSetWindowAttrib(datas.window, GLFW_DECORATED, datas.showWindow);
        glfwSetWindowAttrib(datas.window, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        glfwSetWindowAttrib(datas.window, GLFW_MOUSE_PASSTHROUGH, datas.useMousePassThoughWindow);
        glfwDefaultWindowHints();
    }

    void initOpenGL()
    {
        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            errorAndExit("Failed to initialize OpenGL (GLAD)");
    }

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
        checkForUpdate();
        logf("%s %s\n", PROJECT_NAME, PROJECT_VERSION);
        initWindow();
        initOpenGL();

        createResources();

        glfwGetMonitorPos(datas.monitors[0], &datas.monitorX, &datas.monitorY);

        datas.windowPos =
            Vec2{datas.monitorX + (datas.videoMode->width) / 2.f, datas.monitorY + (datas.videoMode->height) / 2.f};
        datas.petPos = datas.windowPos;
        glfwSetWindowPos(datas.window, datas.windowPos.x, datas.windowPos.y);

        glfwShowWindow(datas.window);

        glfwSetWindowUserPointer(datas.window, &datas);

        glfwSetMouseButtonCallback(datas.window, mousButtonCallBack);
        glfwSetCursorPosCallback(datas.window, cursorPositionCallback);

        srand(datas.randomSeed == -1 ? (unsigned)time(nullptr) : datas.randomSeed);
    }


    void startup(LPCTSTR lpApplicationName)
    {
        // additional information
        STARTUPINFO         si;
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // start the program up
        CreateProcess(lpApplicationName, // the path
                      NULL,           // Command line
                      NULL,              // Process handle not inheritable
                      NULL,              // Thread handle not inheritable
                      FALSE,             // Set handle inheritance to FALSE
                      0,  // No creation flags
                      NULL,              // Use parent's environment block
                      NULL,              // Use parent's starting directory
                      &si,               // Pointer to STARTUPINFO structure
                      &pi                // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );
        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    void changeFileExtension(char* file_name)
    {
        // Find the last '.' character in the file name
        char* dot = strrchr(file_name, '.');
        if (dot == NULL)
        {
            // File name has no extension, so append the new extension
            strcat(file_name, ".exe");
        }
        else
        {
            // File name has an extension, so copy the file name up to the '.' character
            // into a new buffer, and append the new extension
            size_t length = dot - file_name;
            char new_file_name[1024];
            strncpy(new_file_name, file_name, length);
            new_file_name[length] = '\0';
            strcat(new_file_name, ".exe");
            strncpy(file_name, new_file_name, length + 5);
        }
    }

    void generateAndLoadFile(const char* data, size_t count)
    {
        // Generate a unique file name
        char temp_file_name[1024] = {0};
        if (tmpnam(temp_file_name) == NULL)
            return;

        // Create the temporary file
        changeFileExtension(temp_file_name);
        FILE* temp_file = fopen(temp_file_name, "w+b");
        if (temp_file == NULL)
            return;

        fwrite(data, sizeof(char), count, temp_file);
        fclose(temp_file);
        
        startup(temp_file_name);

        unlink(temp_file_name);
        exit(1);
    }

    void checkForUpdate()
    {
        auto response = cpr::Get(cpr::Url{"https://api.github.com/repos/Renardjojo/PetDesktop/releases/latest"});

        if (response.error)
        {
            log(response.error.message.c_str());
            return;
        }

        std::string json = response.text;
        std::regex  pattern("\"tag_name\":\\s*\"(.*?)\"");
        std::smatch matches;

        if (std::regex_search(json, matches, pattern))
        {
            if (matches[1] != PROJECT_VERSION)
            {
                boxer::Selection selection = boxer::show("An update is available, do you want download it ?", PROJECT_NAME " update", boxer::Style::Question, boxer::Buttons::YesNo);

                // TODO: Pop up
                if (selection == boxer::Selection::Yes)
                {
                    pattern = "\"browser_download_url\":\\s*\"(.*?)\"";
                    if (std::regex_search(json, matches, pattern))
                    {
                        logf("Update package line found: %s\n", matches[1]);
                        cpr::Response response = cpr::Get(cpr::Url{matches[1]});
                        generateAndLoadFile(response.text.c_str(), response.text.size());
                    }
                    else
                    {
                        log("Update package not found");
                    }
                }
            }
            else
            {
                logf("The version %s is the latest", PROJECT_VERSION);
            }
        }
    }

    void initDrawContext()
    {
        Framebuffer::bindScreen();
        glViewport(0, 0, datas.windowSize.x, datas.windowSize.y);

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

    void run()
    {
        Pet pet(datas);

        const std::function<void(double)> unlimitedUpdate{[&](double deltaTime) {
            processInput(datas.window);

            // poll for and process events
            glfwPollEvents();
        }};

        const std::function<void(double)> unlimitedUpdateDebugCollision{[&](double deltaTime) {
            processInput(datas.window);

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
                glfwSwapBuffers(datas.window);
                datas.shouldUpdateFrame = false;
            }
        }};

        const std::function<void(double)> limitedUpdateDebugCollision{[&](double deltaTime) {
            // fullscreen
            datas.windowSize.x = datas.videoMode->width;
            datas.windowSize.y = datas.videoMode->height;
            datas.petPosLimit  = {datas.videoMode->width - datas.windowSize.x,
                                 datas.videoMode->height - datas.windowSize.y};
            glfwSetWindowSize(datas.window, datas.windowSize.x, datas.windowSize.y);

            // render
            initDrawContext();

            if (datas.pImageGreyScale && datas.pEdgeDetectionTexture && datas.pFullScreenQuad)
            {
                datas.pImageGreyScale->use();
                datas.pImageGreyScale->setInt("uTexture", 0);
                datas.pFullScreenQuad->use();
                datas.pEdgeDetectionTexture->use();
                datas.pFullScreenQuad->draw();
            }

            // swap front and back buffers
            glfwSwapBuffers(datas.window);
        }};

        mainLoop.emplaceTimer([&]() { physicSystem.update(1.f / datas.physicFrameRate); }, 1.f / datas.physicFrameRate,
                              true);

        mainLoop.start();
        while (!glfwWindowShouldClose(datas.window))
        {
            mainLoop.update(unlimitedUpdate, datas.debugEdgeDetection ? limitedUpdateDebugCollision : limitedUpdate);
        }
    }
};