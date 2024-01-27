#include "Engine/WindowSDL.hpp"

#include "Engine/Graphics/WindowOGL.hpp"
#include "Engine/Log.hpp"
#include "Game/GameData.hpp"
#include "Game/Pet.hpp"

void WindowSDL::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        errorAndExit(std::string("SDL initialization error") + SDL_GetError());

    SDL_GL_LoadLibrary(NULL);

    // Request an OpenGL 4.5 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
}

void WindowSDL::initWindow(GameData& datas)
{
    int windowFlags = SDL_WINDOW_OPENGL;

    if (!datas.showFrameBufferBackground)
        windowFlags |= SDL_WINDOW_TRANSPARENT;
    
    m_isForwardWindow = datas.useForwardWindow;
    if (m_isForwardWindow)
        windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    
    if (!datas.showWindow)
        windowFlags |= SDL_WINDOW_BORDERLESS;

    m_size = {300.f, 300.f};
    m_window = SDL_CreateWindow(PROJECT_NAME, m_size.x, m_size.y, windowFlags);

    m_glcontext = SDL_GL_CreateContext(m_window);
    if (m_glcontext == NULL)
        errorAndExit(std::string("Failed to create OpenGL context: ") + SDL_GetError());

    if (!m_window)
        errorAndExit(std::string("Create Window error") + SDL_GetError());

    int rendererFlags = SDL_RENDERER_ACCELERATED;
    m_renderer        = SDL_CreateRenderer(m_window, NULL, rendererFlags);

    if (!m_renderer)
        errorAndExit(std::string("Failed to create renderer: ") + SDL_GetError());
}

void WindowSDL::postSetupWindow(GameData& datas)
{
    m_useMousePassThrough = datas.useMousePassThoughWindow;
    m_isMousePassThrough  = true;
    SDL_CaptureMouse(m_isMousePassThrough);
    // glfwSetWindowAttrib(window, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    // glfwSetWindowUserPointer(window, &datas);
    // glfwSetMouseButtonCallback(window, mousButtonCallBack);
    // glfwSetCursorPosCallback(window, cursorPositionCallback);
    // glfwSetDropCallback(window, dropCallback);

    SDL_ShowWindow(m_window);
    SDL_SetWindowPosition(m_window, m_position.x, m_position.y);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        errorAndExit("Failed to initialize OpenGL (GLAD)");
}

void WindowSDL::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            m_shouldClose = 1;
            break;
        }
    }
}

/*
void dropCallback(GLFWwindow* window, int count, const char** paths)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));
    datas.droppedFiles.clear();
    for (int i = 0; i < count; i++)
    {
        datas.droppedFiles.emplace_back(paths[i]);
    }
}

void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));
    if (datas.leftButtonEvent == GLFW_PRESS)
    {
        float globalScreenPosX = static_cast<float>(datas.window->getPosition().x + x);
        float globalScreenPosY = static_cast<float>(datas.window->getPosition().y + y);
        datas.deltaCursorPosX += globalScreenPosX - datas.prevCursorPosX;
        datas.deltaCursorPosY += globalScreenPosY - datas.prevCursorPosY;
        datas.prevCursorPosX = globalScreenPosX;
        datas.prevCursorPosY = globalScreenPosY;
        Vec2 delta(datas.deltaCursorPosX, datas.deltaCursorPosY);
        datas.deltasCursorPosBuffer.emplace(GameData::DeltaCursosPosElem{(float)datas.timeAcc, delta});
        datas.deltaCursorAcc += delta;
    }
}

void mousButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        datas.leftButtonEvent = action;

        switch (action)
        {
        case GLFW_PRESS: {
            datas.prevCursorPosX  = static_cast<float>(datas.window->getPosition().x + datas.cursorPos.x);
            datas.prevCursorPosY  = static_cast<float>(datas.window->getPosition().y + datas.cursorPos.y);
            datas.deltaCursorPosX = 0.f;
            datas.deltaCursorPosY = 0.f;
            break;
        }
        case GLFW_RELEASE:
            break;
        default:
            break;
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        datas.rightButtonEvent = action;
        break;
    default:
        break;
    }
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    // Need always capture the mouse position to trigger the pass through
    double cursPosX, cursPosY;
    glfwGetCursorPos(window, &cursPosX, &cursPosY);
    datas.cursorPos = {static_cast<int>(floor(cursPosX)), static_cast<int>(floor(cursPosY))};

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}*/