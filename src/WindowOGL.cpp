#include "Engine/Graphics/WindowOGL.hpp"
#include "Engine/Graphics/FramebufferOGL.hpp"

#include "Engine/Log.hpp"
#include "Game/GameData.hpp"

void Window::init(GameData& datas)
{
    WindowSDL::init();

    initWindow(datas);
    postSetupWindow(datas);

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugMessageCallback, NULL);
#endif
}

void Window::initDrawContext()
{
    Framebuffer::bindScreen();

    glViewport(0, 0, m_size.x, m_size.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
}