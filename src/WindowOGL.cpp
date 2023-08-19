#include "Engine/Graphics/WindowOGL.hpp"
#include "Engine/Graphics/FramebufferOGL.hpp"

#include "Engine/Log.hpp"
#include "Game/GameData.hpp"

void Window::initGraphicAPI()
{
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        errorAndExit("Failed to initialize OpenGL (GLAD)");

    #ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugMessageCallback, NULL);
#endif
}

void Window::init(GameData& datas)
{
    initGLFW();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    initWindow(datas);
    initGraphicAPI();
}

void Window::initDrawContext()
{
    Framebuffer::bindScreen();

    glViewport(0, 0, windowSize.x, windowSize.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
}