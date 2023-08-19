#pragma once

#include "boxer/boxer.h"

#include <cstdarg>
#include <stdio.h>
#include <string>

inline void log(const char* buffer)
{
#if _DEBUG
    // log into console
    fputs(buffer, stderr);
#endif
}

inline void logf(char const* const format, ...)
{
    va_list arglist;
    va_start(arglist, format);
#if _DEBUG
    // log into console
    vfprintf(stderr, format, arglist);
#endif
    va_end(arglist);
}

inline void errorAndExit(const std::string& msg)
{
    boxer::Selection selection =
        boxer::show(msg.c_str(), PROJECT_NAME " error", boxer::Style::Error, boxer::Buttons::OK);
    exit(-1);
}

inline void warning(const std::string& msg)
{
    boxer::show(msg.c_str(), PROJECT_NAME " warning", boxer::Style::Warning, boxer::Buttons::OK);
    exit(-1);
}

#if defined(USE_OPENGL_API)

#include <glad/glad.h>

inline void GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                   const GLchar* msg, const void* data)
{
    const char* _source;
    const char* _type;
    const char* _severity;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

    default:
        _source = "UNKNOWN";
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

    case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

    case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

    default:
        _type = "UNKNOWN";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;

    case GL_DEBUG_SEVERITY_LOW:
        _severity = "LOW";
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;

    default:
        _severity = "UNKNOWN";
        break;
    }

    logf("%d: %s of %s severity, raised from %s: %s\n", id, _type, _severity, _source, msg);
}
#endif
