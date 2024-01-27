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

    m_size   = {1.f, 1.f};
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

    SDL_ShowWindow(m_window);
    SDL_SetWindowPosition(m_window, m_position.x, m_position.y);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        errorAndExit("Failed to initialize OpenGL (GLAD)");
}

void WindowSDL::pollEvents(GameData& datas)
{
    // Need always capture the mouse position to trigger the pass through
    float cursPosX, cursPosY;
    SDL_GetGlobalMouseState(&cursPosX, &cursPosY);
    datas.cursorPos = {static_cast<int>(floor(cursPosX)), static_cast<int>(floor(cursPosY))};

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            m_shouldClose = 1;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (datas.leftButtonEvent == SDL_PRESSED)
            {
                float globalScreenPosX = static_cast<float>(datas.window->getPosition().x + event.motion.x);
                float globalScreenPosY = static_cast<float>(datas.window->getPosition().y + event.motion.x);
                datas.deltaCursorPosX += globalScreenPosX - datas.prevCursorPosX;
                datas.deltaCursorPosY += globalScreenPosY - datas.prevCursorPosY;
                datas.prevCursorPosX = globalScreenPosX;
                datas.prevCursorPosY = globalScreenPosY;
                Vec2 delta(datas.deltaCursorPosX, datas.deltaCursorPosY);
                datas.deltasCursorPosBuffer.emplace(GameData::DeltaCursosPosElem{(float)datas.timeAcc, delta});
                datas.deltaCursorAcc += delta;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                datas.leftButtonEvent = event.button.state;
                datas.prevCursorPosX  = static_cast<float>(datas.window->getPosition().x + datas.cursorPos.x);
                datas.prevCursorPosY  = static_cast<float>(datas.window->getPosition().y + datas.cursorPos.y);
                datas.deltaCursorPosX = 0.f;
                datas.deltaCursorPosY = 0.f;
                break;
            case SDL_BUTTON_RIGHT:
                datas.rightButtonEvent = event.button.state;
                break;
            default:
                break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                datas.leftButtonEvent = event.button.state;
                break;
            case SDL_BUTTON_RIGHT:
                datas.rightButtonEvent = event.button.state;
                break;
            default:
                break;
            }
            break;
        case SDL_EVENT_DROP_FILE:
            if (event.drop.type == SDL_EVENT_DROP_BEGIN)
                datas.droppedFiles.clear();

            datas.droppedFiles.emplace_back(event.drop.source);
            SDL_free(event.drop.source); // Free dropped_filedir memory
            break;
        default:
            break;
        }
    }
}