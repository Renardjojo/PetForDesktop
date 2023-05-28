#pragma once

#include "Engine/WindowGLFW.hpp"

#include <glad/glad.h>

void GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg,
                            const void* data);

class Window : public WindowGLFW
{
protected:
    void initGraphicAPI();

public:
    void init(struct GameData& datas);
};