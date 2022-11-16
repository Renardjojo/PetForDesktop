#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>

#ifdef __linux__
#elif _WIN32
#include "WindowUtility.h"
#else
#endif

#include "Vector2.hpp"
#include "boxer/boxer.h"
#include "yaml-cpp/yaml.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void log(const char* buffer)
{
#if _DEBUG
    // log into console
    fputs(buffer, stderr);
#endif
}

void logf(char const* const format, ...)
{
    va_list arglist;
    va_start(arglist, format);
#if _DEBUG
    // log into console
    vfprintf(stderr, format, arglist);
#endif
    va_end(arglist);
}

void errorAndExit(const std::string& msg)
{
    boxer::Selection selection = boxer::show(msg.c_str(), "PetForDesktop error", boxer::Style::Error, boxer::Buttons::OK);
    exit(-1);
}

void warning(const std::string& msg)
{
    boxer::show(msg.c_str(), "PetForDesktop warning", boxer::Style::Warning, boxer::Buttons::OK);
    exit(-1);
}

class FileReader
{
protected:
    char* buffer = NULL;

public:
    FileReader(const char* filename)
    {
        int     string_size, read_size;
        FILE*   handler;
        errno_t err;

        err = fopen_s(&handler, filename, "rb");
        if (err != 0)
        {
            logf("The file '%s' was not opened\n", filename);
        }

        if (handler)
        {
            // Seek the last byte of the file
            fseek(handler, 0, SEEK_END);
            // Offset from the first to the last byte, or in other words, filesize
            string_size = ftell(handler);
            // go back to the start of the file
            rewind(handler);

            // Allocate a string that can hold it all
            buffer = (char*)malloc(sizeof(char) * (string_size + 1));

            // Read it all in one operation
            read_size = fread(buffer, sizeof(char), string_size, handler);

            // fread doesn't set it so put a \0 in the last position
            // and buffer is now officially a string
            buffer[string_size] = '\0';

            if (string_size != read_size)
            {
                // Something went wrong, throw away the memory and set
                // the buffer to NULL
                free(buffer);
                buffer = NULL;
            }

            // Always remember to close the file.
            fclose(handler);
        }
    }

    ~FileReader()
    {
        free(buffer);
        buffer = NULL;
    }

    const char* get()
    {
        return buffer;
    }
};

class Shader
{
public:
    unsigned int ID;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        logf("Parse files: %s %s\n", vertexPath, fragmentPath);
        FileReader  vertexCodeFile(vertexPath);
        FileReader  fragmentCodeFile(fragmentPath);
        const char* vShaderCode = vertexCodeFile.get();
        const char* fShaderCode = fragmentCodeFile.get();

        // compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        log("Shader compilationd done");
    }

    void use()
    {
        glUseProgram(ID);
    }

    void setBool(const char* name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name), (int)value);
    }

    void setInt(const char* name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }

    void setVec2(const char* name, float v1, float v2) const noexcept
    {
        glUniform2f(glGetUniformLocation(ID, name), v1, v2);
    }

    void setVec4(const char* name, float v1, float v2, float v3, float v4) const noexcept
    {
        glUniform4f(glGetUniformLocation(ID, name), v1, v2, v3, v4);
    }

    void setFloat(const char* name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }

private:
    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(unsigned int shader, const char* type)
    {
        int  success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                errorAndExit(std::string("Shader compilation errorof type") + type + '\n' + infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                errorAndExit(std::string("Program linking errorof type") + type + '\n' + infoLog);
            }
        }
    }
};

class Texture
{
protected:
    unsigned int ID;
    int          width, height;
    int          nbChannels;

public:
    Texture(const char* srcPath, std::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        // load image, create texture and generate mipmaps
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char* data = stbi_load(srcPath, &width, &height, &nbChannels, 0);
        if (data)
        {
            if (nbChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            log("Failed to load texture");
        }
        stbi_image_free(data);
    }

    Texture(void* data, int pxlWidth, int pxlHeight, int channels = 3,
            std ::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        width           = pxlWidth;
        height          = pxlHeight;
        nbChannels      = channels;
        GLenum chanEnum = getChanelEnum();
        glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    }

    Texture(int pxlWidth, int pxlHeight, int channels = 4, std::function<void()> setupCallback = defaultSetupCallBack)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        setupCallback();

        width           = pxlWidth;
        height          = pxlHeight;
        nbChannels      = channels;
        GLenum chanEnum = getChanelEnum();

        glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, chanEnum, GL_UNSIGNED_BYTE, 0);
    }

    static void defaultSetupCallBack()
    {
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    ~Texture()
    {
        glDeleteTextures(1, &ID);
    }

    void use() const
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    int getHeight() const
    {
        return height;
    }

    int getWidth() const
    {
        return width;
    }

    int getChannelCount() const
    {
        return nbChannels;
    }

    int getID() const
    {
        return ID;
    }

    GLenum getChanelEnum()
    {
        return nbChannels > 3 ? GL_RGBA : nbChannels == 3 ? GL_RGB : nbChannels == 2 ? GL_RG : GL_RED;
    }

    // Warning, texture need to be binding before
    void getPixels(std::vector<unsigned char>& data)
    {
        int pixelsCount = width * height * nbChannels;
        data.reserve(pixelsCount);

        for (size_t i = 0; i < pixelsCount; i++)
        {
            data.emplace_back(0);
        }

        glGetTextureImage(ID, 0, getChanelEnum(), GL_UNSIGNED_BYTE, pixelsCount * sizeof(unsigned char), &data[0]);
    }
};

class Framebuffer
{
protected:
    unsigned int ID;

public:
    Framebuffer()
    {
        glGenFramebuffers(1, &ID);
    }

    ~Framebuffer()
    {
        glDeleteFramebuffers(1, &ID);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    void attachTexture(const Texture& texture)
    {
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getID(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            log("Framebuffer error");
        }
    }

    static void bindScreen()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

// TODO full screen Triangle
class ScreenSpaceQuad
{
protected:
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;

public:
    ScreenSpaceQuad(float minPos = -1.f, float maxPos = 1.f)
    {
        // TODO: Pos vec2 ?
        float vertices[] = {
            // positions        // texture coords
            maxPos, maxPos, 0.0f, 1.0f, 1.0f, // top right
            maxPos, minPos, 0.0f, 1.0f, 0.0f, // bottom right
            minPos, minPos, 0.0f, 0.0f, 0.0f, // bottom left
            minPos, maxPos, 0.0f, 0.0f, 1.0f  // top left
        };

        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    ~ScreenSpaceQuad()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void use()
    {
        glBindVertexArray(VAO);
    }

    void draw()
    {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};

struct GameData
{
    // Window and monitor
    GLFWwindow*        window       = nullptr;
    GLFWmonitor**      monitors     = nullptr;
    const GLFWvidmode* videoMode    = nullptr;
    int                monitorCount = 0, monitorX = 0, monitorY = 0;
    Vec2               petPos  = {0.f, 0.f};
    Vec2i              petSize = {0.f, 0.f};

    Vec2i windowExt    = {0.f, 0.f};
    Vec2i windowMinExt = {0.f, 0.f};

    Vec2i windowSize  = {0.f, 0.f};
    Vec2i windowPos   = {0.f, 0.f};
    Vec2i petPosLimit = {0, 0};

    // Resources
    std::unique_ptr<Framebuffer> pFramebuffer = nullptr;

    std::unique_ptr<Shader> pImageShader       = nullptr;
    std::unique_ptr<Shader> pImageGreyScale    = nullptr;
    std::unique_ptr<Shader> pSpriteSheetShader = nullptr;
    std::vector<Shader>     edgeDetectionShaders; // Sorted by pass

    std::unique_ptr<Texture> pCollisionTexture     = nullptr;
    std::unique_ptr<Texture> pEdgeDetectionTexture = nullptr;

    std::unique_ptr<ScreenSpaceQuad> pUnitFullScreenQuad = nullptr;
    std::unique_ptr<ScreenSpaceQuad> pFullScreenQuad     = nullptr;

    // Inlog
    float prevCursorPosX  = 0;
    float prevCursorPosY  = 0;
    float deltaCursorPosX = 0;
    float deltaCursorPosY = 0;
    int   leftButtonEvent = 0;

    // Settings
    int FPS        = 0;
    int scale      = 0;
    int randomSeed = 0;

    // Physic
    int  physicFrameRate = 60;
    Vec2 velocity        = {0.f, 0.f};
    // This value is not changed by the physic system. Usefull for movement. Friction is applied to this value
    Vec2  continusVelocity                = {0.f, 0.f};
    Vec2  gravity                         = {0.f, 0.f};
    Vec2  gravityDir                      = {0.f, 0.f};
    float bounciness                      = 0.f;
    float friction                        = 0.f;
    float continusCollisionMaxSqrVelocity = 0.f;
    float collisionPixelRatioStopMovement = 0.f;
    float isGroundedDetection             = 0.f;
    int   footBasasementWidth             = 1;
    int   footBasasementHeight            = 1;
    bool  isGrounded                      = false;

    // Animation
    bool  side                 = true; // false left / true right
    bool  isGrab               = false;

    // Time
    double timeAcc = 0.f;

    // Window
    bool showWindow                = false;
    bool showFrameBufferBackground = false;
    bool useFowardWindow           = true;
    bool useMousePassThoughWindow  = true;

    // Debug
    bool debugEdgeDetection = false;
};

// Dont forgot to use glDeleteTextures(1, &ID); after usage
void createTextureFromScreenShoot(unsigned int& ID)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ScreenShoot screenshoot(0, 0, 400, 400);

    int                      width  = 400;
    int                      height = 400;
    const ScreenShoot::Data& data   = screenshoot.get();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data.bits);
}

float gammaToLinearByte(char gammaValue)
{
    return std::pow(gammaValue / 255.f, 2.2);
}

void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    if (datas.leftButtonEvent == GLFW_PRESS)
    {
        datas.deltaCursorPosX = x - datas.prevCursorPosX;
        datas.deltaCursorPosY = y - datas.prevCursorPosY;
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
        case GLFW_PRESS:
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            datas.prevCursorPosX  = floor(x);
            datas.prevCursorPosY  = floor(y);
            datas.deltaCursorPosX = 0.f;
            datas.deltaCursorPosY = 0.f;
            datas.isGrounded      = false;
            break;
        case GLFW_RELEASE:
            datas.velocity = Vec2{datas.deltaCursorPosX / datas.FPS, datas.deltaCursorPosY / datas.FPS};
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void processMousePassTHoughWindow(GLFWwindow* window, GameData& datas)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    const Vec2 localWinPos             = datas.petPos - datas.windowPos;
    const bool isCursorInsidePetWindow = xPos > localWinPos.x && yPos > localWinPos.y &&
                                         xPos < localWinPos.x + (float)datas.petSize.x &&
                                         yPos < localWinPos.y + (float)datas.petSize.y;

    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, !isCursorInsidePetWindow);
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (datas.useMousePassThoughWindow)
        processMousePassTHoughWindow(window, datas);
}

int randNum(int min, int max)
{
    return (rand() % (((max) + 1) - (min))) + (min);
}

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
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

class SpriteSheet : public Texture
{
protected:
    int tileCount;

public:
    SpriteSheet(const char* srcPath) : Texture(srcPath)
    {
        tileCount = width / height;
    }

    void useSection(GameData& data, Shader& shader, int idSection, bool hFlip = false)
    {
        data.petSize.x    = height * data.scale; // use height because texture is horizontal sprite sheet only
        data.petSize.y    = height * data.scale;
        data.windowSize.x = data.petSize.x + data.windowExt.x + data.windowMinExt.x;
        data.windowSize.y = data.petSize.y + data.windowExt.y + data.windowMinExt.y;
        data.petPosLimit  = {data.videoMode->width - data.petSize.x, data.videoMode->height - data.petSize.y};
        glfwSetWindowSize(data.window, data.windowSize.x, data.windowSize.y);

        float       hScale  = 1.f / tileCount;
        const float vScale  = 1.f; // This field can be used
        float       hOffSet = idSection / (float)tileCount;
        const float vOffset = 0.f; // This field can be used

        Vec2 clipSpacePos  = Vec2::remap(static_cast<Vec2i>(data.petPos), data.windowPos,
                                        data.windowPos + data.windowSize, Vec2{0, 1}, Vec2{1, 0});          // [-1, 1]
        Vec2 clipSpaceSize = Vec2::remap(data.petSize, Vec2{0, 0}, data.windowSize, Vec2{0, 0}, Vec2{1, 1}); // [0, 1]

        // In shader, based on bottom left instead of upper left
        clipSpacePos.y -= clipSpaceSize.y;

        if (hFlip)
        {
            hOffSet += hScale;
            hScale *= -1;
        }

        shader.use();
        shader.setVec4("uScaleOffSet", hScale, vScale, hOffSet, vOffset);
        shader.setVec4("uClipSpacePosSize", clipSpacePos.x, clipSpacePos.y, clipSpaceSize.x, clipSpaceSize.y);
        use();
    }

    int getTileCount() const
    {
        return tileCount;
    }
};

class Setting
{
public:
    Setting(const char* path, GameData& data)
    {
        YAML::Node animGraph = YAML::LoadFile(path);
        if (!animGraph)
        {
            errorAndExit(std::string("Could not find setting file here: ") + path);
        }

        std::string section;
        {
            section                 = "Game";
            YAML::Node nodesSection = animGraph[section];
            if (!nodesSection)
                errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

            data.FPS        = std::max(nodesSection["FPS"].as<int>(), 1);
            data.scale      = std::max(nodesSection["Scale"].as<int>(), 1);
            data.randomSeed = nodesSection["RandomSeed"].as<int>();
        }

        {
            section = "Physic";
            YAML::Node nodesSection = animGraph[section];
            if (!nodesSection)
                errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

            data.physicFrameRate = std::max(nodesSection["PhysicFrameRate"].as<int>(), 0);
            data.bounciness      = std::clamp(nodesSection["Bounciness"].as<float>(), 0.f, 1.f);
            data.gravity =
                Vec2{nodesSection["GravityX"].as<float>(), nodesSection["GravityY"].as<float>()};
            data.gravityDir = data.gravity.normalized();
            data.friction                        = std::clamp(nodesSection["Friction"].as<float>(), 0.f, 1.f);
            data.continusCollisionMaxSqrVelocity =
                std::max(nodesSection["ContinusCollisionMaxVelocity"].as<float>(), 0.f);
            data.continusCollisionMaxSqrVelocity *= data.continusCollisionMaxSqrVelocity;
            data.footBasasementWidth  = std::max(nodesSection["FootBasasementWidth"].as<int>(), 2);
            data.footBasasementHeight            = std::max(nodesSection["FootBasasementHeight"].as<int>(), 2);
            data.collisionPixelRatioStopMovement =
                std::clamp(nodesSection["CollisionPixelRatioStopMovement"].as<float>(), 0.f, 1.f);
            data.isGroundedDetection = std::max(nodesSection["IsGroundedDetection"].as<float>(), 0.f);
        }

        {
            section                 = "Window";
            YAML::Node nodesSection = animGraph[section];
            if (!nodesSection)
                errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

            data.showWindow                = nodesSection["ShowWindow"].as<bool>();
            data.showFrameBufferBackground = nodesSection["ShowFrameBufferBackground"].as<bool>();
            data.useFowardWindow           = nodesSection["UseFowardWindow"].as<bool>();
            data.useMousePassThoughWindow  = nodesSection["UseMousePassThoughWindow"].as<bool>();
        }

        {
            section                 = "Debug";
            YAML::Node nodesSection = animGraph[section];
            if (!nodesSection)
                errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

            data.debugEdgeDetection = nodesSection["ShowEdgeDetection"].as<bool>();
        }
    }
};

class PhysicSystem
{
protected:
    GameData& data;

public:
    PhysicSystem(GameData& data) : data{data}
    {
    }

    bool checkIsGrounded()
    {
        return std::abs(data.gravityDir.dot(data.velocity)) < data.isGroundedDetection;
    }

    void computeMonitorCollisions()
    {
        if (data.petPos.x < 0.f)
        {
            data.petPos.x = 0.f;
            data.velocity = data.velocity.reflect(Vec2::right()) * data.bounciness;
        }

        if (data.petPos.y < 0.f)
        {
            data.petPos.y = 0.f;
            data.velocity = data.velocity.reflect(Vec2::down()) * data.bounciness;
        }

        if (data.petPos.x > data.petPosLimit.x)
        {
            data.petPos.x = data.petPosLimit.x;
            data.velocity = data.velocity.reflect(Vec2::left()) * data.bounciness;
        }

        if (data.petPos.y > data.petPosLimit.y)
        {
            data.petPos.y = data.petPosLimit.y;
            data.velocity = data.velocity.reflect(Vec2::up()) * data.bounciness;

            // check if is grounded
            data.isGrounded = checkIsGrounded();
            data.velocity *= !data.isGrounded; // reset velocity if is grounded
        }
    }

    void updateCollisionTexture(const Vec2 prevToNewWinPos)
    {
        int screenShootPosX, screenShootPosY, screenShootSizeX, screenShootSizeY;
        if (data.debugEdgeDetection)
        {
            screenShootPosX  = 0.f;
            screenShootPosY  = 0.f;
            screenShootSizeX = data.windowSize.x;
            screenShootSizeY = data.windowSize.y;
        }
        else
        {
            const float xPadding = prevToNewWinPos.x < 0.f ? prevToNewWinPos.x : 0.f;
            const float yPadding = prevToNewWinPos.y < 0.f ? prevToNewWinPos.y : 0.f;

            screenShootPosX  = data.petPos.x + data.petSize.x / 2.f + xPadding - data.footBasasementWidth / 2.f;
            screenShootPosY  = data.petPos.y + data.petSize.y + 1 + yPadding - data.footBasasementHeight / 2.f;
            screenShootSizeX = abs(prevToNewWinPos.x) + data.footBasasementWidth;
            screenShootSizeY = abs(prevToNewWinPos.y) + data.footBasasementHeight;
        }

        ScreenShoot              screenshoot(screenShootPosX, screenShootPosY, screenShootSizeX, screenShootSizeY);
        const ScreenShoot::Data& pxlData = screenshoot.get();

        data.pCollisionTexture     = std::make_unique<Texture>(pxlData.bits, pxlData.width, pxlData.height, 4);
        data.pEdgeDetectionTexture = std::make_unique<Texture>(pxlData.width, pxlData.height, 4);

        glDisable(GL_BLEND);
        glViewport(0, 0, pxlData.width, pxlData.height);

        if (data.edgeDetectionShaders.size() == 1)
        {
            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pEdgeDetectionTexture);

            data.edgeDetectionShaders[0].use();
            data.edgeDetectionShaders[0].setInt("uTexture", 0);
            data.edgeDetectionShaders[0].setVec2("resolution", pxlData.width, pxlData.height);
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();
        }
        else
        {
            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pCollisionTexture);

            data.edgeDetectionShaders[0].use();
            data.edgeDetectionShaders[0].setInt("uTexture", 0);
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();

            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pEdgeDetectionTexture);

            data.edgeDetectionShaders[1].use();
            data.edgeDetectionShaders[1].setInt("uTexture", 0);
            data.edgeDetectionShaders[1].setVec2("resolution", pxlData.width, pxlData.height);
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();
        }
    }

    bool processContinuousCollision(const Vec2 prevToNewWinPos, Vec2& newPos)
    {
        // Main idear is the we will take a screen shoot of the dimension of the velocity vector (depending on it's
        // magnitude)
        // Thanks to this texture, we will iterate on pixel base on velocity vector to check collision
        // Screen shoot will be post processed with edge detection alogorythm to have only white and bblack values.
        // White will be the collision

        if (prevToNewWinPos.sqrLength() == 0.f)
            return false;

        updateCollisionTexture(prevToNewWinPos);

        std::vector<unsigned char> pixels;
        data.pEdgeDetectionTexture->use();
        data.pEdgeDetectionTexture->getPixels(pixels);

        int dataPerPixel = data.pEdgeDetectionTexture->getChannelCount();

        bool iterationOnX = abs(prevToNewWinPos.x) > abs(prevToNewWinPos.y);
        Vec2 prevToNewWinPosDir;

        if (iterationOnX)
        {
            prevToNewWinPosDir = prevToNewWinPos / sqrtf(prevToNewWinPos.x * prevToNewWinPos.x);
        }
        else
        {
            prevToNewWinPosDir = prevToNewWinPos / sqrtf(prevToNewWinPos.y * prevToNewWinPos.y);
        }

        int width  = data.pEdgeDetectionTexture->getWidth();
        int height = data.pEdgeDetectionTexture->getHeight();

        float row    = prevToNewWinPosDir.y < 0.f ? height - data.footBasasementHeight : 0.f;
        float column = prevToNewWinPosDir.x < 0.f ? width - data.footBasasementWidth : 0.f;

        int iterationCount = iterationOnX ? width - data.footBasasementWidth : height - data.footBasasementHeight;
        for (int i = 0; i < iterationCount + 1; i++)
        {
            float count = 0;

            for (int y = 0; y < data.footBasasementHeight; y++)
            {
                for (int x = 0; x < data.footBasasementWidth; x++)
                {
                    // flip Y and find index
                    int rowFlipped = height - 1 - (int)row - y;
                    int index      = (rowFlipped * width + (int)column + x) * dataPerPixel;
                    count += pixels[index] == 255;
                }
            }
            count /= data.footBasasementWidth * data.footBasasementHeight;

            if (count > data.collisionPixelRatioStopMovement)
            {
                newPos = data.petPos + Vec2(column, row);
                return true;
            }
            row += prevToNewWinPosDir.y;
            column += prevToNewWinPosDir.x;
        }
        return false;
    }

    bool CatpureScreenCollision(const Vec2 prevToNewWinPos, Vec2& newPos)
    {
        return processContinuousCollision(prevToNewWinPos, newPos);
    }

    void update(double deltaTime)
    {
        // Apply gravity if not selected
        if (data.leftButtonEvent != GLFW_PRESS)
        {
            // Acc = Sum of force / Mass
            // G is already an acceleration
            const Vec2 acc = data.gravity * !data.isGrounded;

            // V = Acc * Time
            data.velocity += acc * deltaTime;

            // Evaluate pixel distance based on dpi and monitor size
            // TODO: bake it
            int width_mm, height_mm;
            glfwGetMonitorPhysicalSize(data.monitors[0], &width_mm, &height_mm);

            const vec2 pixelPerMeter{(float)data.videoMode->width / (width_mm * 0.001f),
                                     (float)data.videoMode->height / (height_mm * 0.001f)};

            const Vec2 prevWinPos = data.petPos;
            // Pos = PrevPos + V * Time
            const Vec2 newWinPos = data.petPos + ((data.continusVelocity + data.velocity) * (1.f - data.friction) *
                                                  pixelPerMeter * deltaTime);
            const Vec2 prevToNewWinPos = newWinPos - prevWinPos;
            if ((prevToNewWinPos.sqrLength() <= data.continusCollisionMaxSqrVelocity && prevToNewWinPos.y > 0.f) ||
                data.debugEdgeDetection)
            {
                Vec2 newPos;
                if (CatpureScreenCollision(prevToNewWinPos, newPos))
                {
                    Vec2 collisionPos = newPos;
                    data.petPos       = collisionPos;
                    data.velocity     = data.velocity.reflect(Vec2::up()) * data.bounciness;

                    // check if is grounded
                    data.isGrounded = checkIsGrounded();
                    data.velocity *= !data.isGrounded; // reset velocity if is grounded
                }
                else
                {
                    data.petPos = newWinPos;
                }
            }
            else
            {
                // Update is grounded
                if (data.isGrounded)
                {
                    Vec2 newPos;
                    Vec2 footBasement((float)data.footBasasementWidth, (float)data.footBasasementHeight);
                    data.isGrounded = CatpureScreenCollision(footBasement, newPos);
                }

                data.petPos = newWinPos;
            }

            // Apply monitor collision
            computeMonitorCollisions();
        }
        else
        {
            data.petPos += Vec2{data.deltaCursorPosX, data.deltaCursorPosY};
            data.deltaCursorPosX = 0;
            data.deltaCursorPosY = 0;
        }

        data.windowPos.x = data.petPos.x - data.windowMinExt.x;
        data.windowPos.y = data.petPos.y - data.windowMinExt.y;
        glfwSetWindowPos(data.window, data.windowPos.x, data.windowPos.y);
    }
};

struct TimerTask
{
    std::function<void()> task        = nullptr;
    double                localTimer  = 0.; // if current time egal 1s and local timer egal 0.5 global time egal 1.5
    double                globalTimer = 0.;
    bool                  isLooping   = false;

    TimerTask(const std::function<void()>& task = nullptr, double localTimer = .0, double globalTimer = .0,
              bool isLooping = false)
        : task{task}, localTimer{localTimer}, globalTimer{globalTimer}, isLooping{isLooping}
    {
    }

    bool operator>(const TimerTask& other) const noexcept
    {
        return globalTimer > other.globalTimer;
    }
};

class TimeManager
{
protected:
    double m_time     = glfwGetTime();
    double m_tempTime = m_time;

    double    m_timeAccLoop    = 0.;
    double    m_deltaTime      = 0.;
    double    m_fixedDeltaTime = 1. / 60.;
    GameData& datas;

    std::priority_queue<TimerTask, std::vector<TimerTask>, std::greater<TimerTask>> m_timerQueue;

public:
    TimeManager(GameData& data) : m_fixedDeltaTime{1. / data.FPS}, datas{data}
    {
    }

    // improve first frame accurancy
    void start()
    {
        m_time     = glfwGetTime();
        m_tempTime = m_time;
    }

    inline void emplaceTimer(std::function<void()> functionToExecute, double delay, bool isLooping = false) noexcept
    {
        m_timerQueue.emplace(functionToExecute, delay, delay + datas.timeAcc, isLooping);
    }

    void update(std::function<void(double deltaTime)> unlimitedUpdateFunction,
                std::function<void(double deltaTime)> limitedUpdateFunction)
    {
        /*unfixed update*/
        unlimitedUpdateFunction(m_deltaTime);

        /*Prepar the next frame*/
        m_tempTime  = glfwGetTime();
        m_deltaTime = m_tempTime - m_time;
        m_time      = m_tempTime;

        // This is temporary
        if (m_deltaTime > 0.25)
            m_deltaTime = 0.25;

        /*Add accumulator*/
        datas.timeAcc += m_deltaTime;
        datas.timeAcc *= !isinf(datas.timeAcc); // reset if isInf (avoid conditionnal jump)

        /*Fixed update*/
        m_timeAccLoop += m_deltaTime;

        while (m_timeAccLoop >= m_fixedDeltaTime)
        {
            limitedUpdateFunction(m_fixedDeltaTime);
            m_timeAccLoop -= m_fixedDeltaTime;
        }

        while (!m_timerQueue.empty() && m_timerQueue.top().globalTimer <= datas.timeAcc)
        {
            const TimerTask& timerTask = m_timerQueue.top();
            timerTask.task();

            if (timerTask.isLooping)
            {
                emplaceTimer(timerTask.task, timerTask.localTimer, timerTask.isLooping);
            }
            m_timerQueue.pop();
        }
    }
};

class SpriteAnimator
{
protected:
    SpriteSheet* pSheet = nullptr;
    float        timer;
    bool         loop;
    bool         isEnd;
    int          frameRate;

    int indexCurrentAnimSprite;

public:
    void play(SpriteSheet& inSheet, bool inLoop, int inFrameRate)
    {
        pSheet                 = &inSheet;
        loop                   = inLoop;
        frameRate              = inFrameRate;
        indexCurrentAnimSprite = 0;
        timer                  = 0.f;
        isEnd                  = false;
    }

    void update(double deltaTime)
    {
        if (!isDone())
        {
            timer += deltaTime;
            indexCurrentAnimSprite = timer * frameRate;

            if (indexCurrentAnimSprite >= pSheet->getTileCount())
            {
                if (loop)
                {

                    indexCurrentAnimSprite = 0;
                    timer -= pSheet->getTileCount() / (float)frameRate;
                }
                else
                {
                    indexCurrentAnimSprite = pSheet->getTileCount() - 1;
                    isEnd                  = true;
                }
            }
        }
    }

    void draw(GameData& datas, Shader& shader, bool donthFlip)
    {
        if (pSheet != nullptr)
            pSheet->useSection(datas, shader, indexCurrentAnimSprite, !donthFlip);
    }

    bool isDone() const
    {
        return pSheet == nullptr || isEnd;
    }
};

class StateMachine
{
public:
    struct Node
    {
        struct Transition
        {
            Node*                 pOwner;
            std::shared_ptr<Node> to;

            virtual void onEnter(GameData& blackBoard){};
            virtual void onUpdate(GameData& blackBoard, double dt){};
            virtual void onExit(GameData& blackBoard){};

            virtual bool canTransition(GameData& blackBoard)
            {
                return false;
            };
        };

        std::vector<std::shared_ptr<Transition>> transitions;

        void AddTransition(std::shared_ptr<Transition> transition)
        {
            transition->pOwner = this;
            transitions.emplace_back(transition);
        }

        virtual void onEnter(GameData& blackBoard)
        {
            for (auto&& transition : transitions)
            {
                transition->onEnter(blackBoard);
            }
        };
        virtual void onUpdate(GameData& blackBoard, double dt)
        {
            for (auto&& transition : transitions)
            {
                transition->onUpdate(blackBoard, dt);
            }
        };
        virtual void onExit(GameData& blackBoard)
        {
            for (auto&& transition : transitions)
            {
                transition->onExit(blackBoard);
            }
        };
    };

protected:
    std::shared_ptr<Node> pCurrentNode = nullptr;

    GameData& blackBoard;

public:
    StateMachine(GameData& inBlackBoard) : blackBoard{inBlackBoard}
    {
    }

    void init(const std::shared_ptr<Node>& initialState)
    {
        assert(initialState != nullptr);
        pCurrentNode = initialState;
        pCurrentNode->onEnter(blackBoard);
    }

    void update(double dt)
    {
        assert(pCurrentNode != nullptr);

        pCurrentNode->onUpdate(blackBoard, dt);

        for (auto&& pNodeTransition : pCurrentNode->transitions)
        {
            assert(pNodeTransition != nullptr);

            if (pNodeTransition->canTransition(blackBoard))
            {
                pCurrentNode->onExit(blackBoard);

                assert(pNodeTransition->to != nullptr);
                pCurrentNode = pNodeTransition->to;
                pCurrentNode->onEnter(blackBoard);
                break;
            }
        }
    }
};

class AnimationNode : public StateMachine::Node
{
protected:
    SpriteAnimator& spriteAnimator;
    SpriteSheet&    spriteSheets;
    int             frameRate;
    bool            loop;

public:
    AnimationNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop = true)
        : spriteAnimator{inSpriteAnimator}, spriteSheets{inSpriteSheets}, frameRate{inFrameRate}, loop{inLoop}
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        StateMachine::Node::onEnter(blackBoard);
        spriteAnimator.play(spriteSheets, loop, frameRate);
    }

    void onUpdate(GameData& blackBoard, double dt) override
    {
        StateMachine::Node::onUpdate(blackBoard, dt);
        spriteAnimator.update(dt);
    }

    void onExit(GameData& blackBoard) override
    {
        StateMachine::Node::onExit(blackBoard);
    }

    bool IsAnimationDone()
    {
        return spriteAnimator.isDone();
    }
};

class PetJumpNode : public AnimationNode
{
    Vec2  baseDir = {0.f, 0.f};
    float vThrust = 0.f;
    float hThrust = 0.f;

public:
    PetJumpNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, Vec2 inBaseDir,
                float inVThrust, float inHThrust)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, false), baseDir{inBaseDir}, vThrust{inVThrust},
          hThrust{inHThrust}
    {
    }

    void onUpdate(GameData& blackBoard, double dt) override
    {
        AnimationNode::onUpdate(blackBoard, dt);

        if (spriteAnimator.isDone()) // Enter only for jump begin because don't loop.
        {
            blackBoard.velocity += baseDir * (blackBoard.side * 2 - 1) * hThrust - blackBoard.gravity * vThrust;
            blackBoard.isGrounded = false;
        }
    }
};

class GrabNode : public AnimationNode
{
public:
    GrabNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, bool inLoop)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop)
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        AnimationNode::onEnter(blackBoard);
        blackBoard.isGrab = true;
    }

    void onExit(GameData& blackBoard) override
    {
        AnimationNode::onExit(blackBoard);
        blackBoard.isGrab = false;
    }
};

class PetWalkNode : public AnimationNode
{
    Vec2  baseDir = {0.f, 0.f};
    float thrust  = 0.f;

public:
    PetWalkNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate, Vec2 inRigghtDir,
                float inThrust, bool inLoop = true)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop), baseDir{inRigghtDir}, thrust{inThrust}
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        AnimationNode::onEnter(blackBoard);
        blackBoard.side = randNum(0, 1);
        blackBoard.continusVelocity += baseDir * (blackBoard.side * 2 - 1) * thrust;
    }

    void onExit(GameData& blackBoard) override
    {
        AnimationNode::onExit(blackBoard);
        blackBoard.continusVelocity -= baseDir * (blackBoard.side * 2 - 1) * thrust;
    }
};

struct AnimationEndTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return static_cast<AnimationNode*>(pOwner)->IsAnimationDone();
    };
};

struct IsGroundedTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return blackBoard.isGrounded;
    };
};

struct IsNotGroundedTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return !blackBoard.isGrounded;
    };
};

struct RandomDelayTransition : public StateMachine::Node::Transition
{
protected:
    float delay = 0.f;
    float timer = 0.f;

    int baseDelay_ms = 0;
    int interval_ms  = 0;

public:
    RandomDelayTransition(int inBaseDelay_ms, int inInterval_ms)
        : baseDelay_ms{inBaseDelay_ms}, interval_ms{inInterval_ms}
    {
    }

    bool canTransition(GameData& blackBoard) final
    {
        if (timer >= delay)
            return true;
        return false;
    };

    void onEnter(GameData& blackBoard) final
    {
        timer = 0;
        delay = baseDelay_ms + randNum(-interval_ms, interval_ms);
        delay *= 0.001; // to seconde
    }

    void onUpdate(GameData& blackBoard, double dt) final
    {
        timer += dt;
    }
};

struct RandomDelayTransitionMultipleExit : public RandomDelayTransition
{
public:
    struct NodeChanceToEnter
    {
        std::shared_ptr<StateMachine::Node> node;
        unsigned int                        chanceToEnter;
    };

protected:
    std::vector<NodeChanceToEnter> m_nodeChanceToEnterBuffer;
    unsigned int                   m_total = 0;

public:
    RandomDelayTransitionMultipleExit(int inBaseDelay_ms, int inInterval_ms,
                                      const std::vector<NodeChanceToEnter>& nodeChanceToEnterBuffer)
        : RandomDelayTransition(inBaseDelay_ms, inInterval_ms), m_nodeChanceToEnterBuffer{nodeChanceToEnterBuffer}
    {
        for (const NodeChanceToEnter& nodeChanceToEnter : m_nodeChanceToEnterBuffer)
        {
            m_total += nodeChanceToEnter.chanceToEnter;
        }
    }

    void onExit(GameData& blackBoard) final
    {
        int score = randNum(1, m_total);

        unsigned int accScore = 0;
        for (const NodeChanceToEnter& nodeChanceToEnter : m_nodeChanceToEnterBuffer)
        {
            accScore += nodeChanceToEnter.chanceToEnter;

            if (accScore >= score)
            {
                to = nodeChanceToEnter.node;
                break;
            }
        }
    }
};

struct StartLeftClicTransition : public StateMachine::Node::Transition
{
    bool canTransition(GameData& blackBoard) final
    {
        return blackBoard.leftButtonEvent == GLFW_PRESS;
    };
};

struct EndLeftClicTransition : public StateMachine::Node::Transition
{
protected:
    bool leftWasPressed = false;

public:
    bool canTransition(GameData& blackBoard) final
    {
        if (blackBoard.leftButtonEvent == GLFW_PRESS)
        {
            leftWasPressed = true;
        }

        if (blackBoard.leftButtonEvent != GLFW_PRESS && leftWasPressed)
        {
            leftWasPressed = false;
            return true;
        }
        return false;
    };
};

class Pet
{
protected:
    enum class ESide
    {
        left,
        right
    };

    std::map<std::string, SpriteSheet> spriteSheets;
    ESide                              side{ESide::left};

    GameData& datas;

    StateMachine   animator;
    SpriteAnimator spriteAnimator;
    int            indexCurrentAnimSprite = 0;
    bool           loopCurrentAnim;

    bool        leftWasPressed = false;
    std::string spritesPath    = RESOURCE_PATH "sprites/";

public:
    Pet(GameData& data) : datas{data}, animator{data}
    {
        parseAnimationGraph();
    }

    SpriteSheet& getOrAddSpriteSheet(const char* file)
    {
        auto it = spriteSheets.find(file);
        if (it != spriteSheets.end())
        {
            return it->second;
        }
        else
        {
            return spriteSheets.emplace(file, (spritesPath + file).c_str()).first->second;
        }
    }

    void parseAnimationGraph()
    {
        YAML::Node  animGraph = YAML::LoadFile(RESOURCE_PATH "setting/animation.yaml");

        // Init nodes
        std::map<std::string, std::shared_ptr<StateMachine::Node>> nodes;

        YAML::Node nodesSection = animGraph["Nodes"];
        if (!nodesSection)
            errorAndExit("Cannot find \"Nodes\" in animation.yaml");

        for (YAML::const_iterator it = nodesSection.begin(); it != nodesSection.end(); ++it)
        {
            std::string title = it->first.Scalar();
            // TODO hash
            if (title == "AnimationNode")
            {
                if (!AddBasicNode<AnimationNode>(it->second, nodes))
                    continue;
            }
            else if (title == "GrabNode")
            {
                if (!AddBasicNode<GrabNode>(it->second, nodes))
                    continue;
            }
            else if (title == "PetWalkNode")
            {
                if (!AddWalkNode(it->second, nodes))
                    continue;
            }
            else if (title == "PetJumpNode")
            {
                if (!AddJumpNode(it->second, nodes))
                    continue;
            }
            else
            {
                warning(std::string("Node with name ") + title + " isn't implemented and is skiped");
            }
        }

        // Init transitions
        YAML::Node transitionsSection = animGraph["Transitions"];
        if (!transitionsSection)
            errorAndExit("Cannot find \"Transitions\" in animation.yaml");

        for (YAML::const_iterator it = transitionsSection.begin(); it != transitionsSection.end(); ++it)
        {
            std::string title = it->first.Scalar();
            // TODO hash
            if (title == "StartLeftClicTransition")
            {
                if (!AddBasicTransition<StartLeftClicTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "RandomDelayTransitionMultipleExit")
            {
                if (!AddRandomDelayTransitionMultipleExit(it->second, nodes))
                    continue;
            }
            else if (title == "IsNotGroundedTransition")
            {
                if (!AddBasicTransition<IsNotGroundedTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "RandomDelayTransition")
            {
                if (!AddRandomDelayTransition(it->second, nodes))
                    continue;
            }
            else if (title == "AnimationEndTransition")
            {
                if (!AddBasicTransition<AnimationEndTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "EndLeftClicTransition")
            {
                if (!AddBasicTransition<EndLeftClicTransition>(it->second, nodes))
                    continue;
            }
            else if (title == "IsGroundedTransition")
            {
                if (!AddBasicTransition<IsGroundedTransition>(it->second, nodes))
                    continue;
            }
            else
            {
                warning(std::string("Transition with name ") + title + " isn't implemented and is skiped");
            }
        }

        // First node
        YAML::Node firstNode = animGraph["FirstNode"];
        if (!firstNode)
            errorAndExit("Cannot find \"FirstNode\" in animation.yaml");

        // Start state machine
        std::string firstNodeName = firstNode.as<std::string>();
        animator.init(std::static_pointer_cast<StateMachine::Node>(nodes[firstNodeName]));
    }

    template <typename T>
    bool AddBasicNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 4)
        {
            warning("YAML error: node invalid");
            return false;
        }
        std::string        nodeName    = node["name"].as<std::string>();
        SpriteSheet&       spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<T> p_node =
            std::make_shared<T>(spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["loop"].as<bool>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddWalkNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 6)
        {
            warning("YAML error: PetWalkNode invalid");
            return false;
        }
        std::string                  nodeName    = node["name"].as<std::string>();
        SpriteSheet&                 spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<PetWalkNode> p_node      = std::make_shared<PetWalkNode>(
            spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["direction"].as<Vec2>(),
            node["thrust"].as<float>(), node["loop"].as<bool>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    bool AddJumpNode(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 6)
        {
            warning("YAML error: PetJumpNode invalid");
            return false;
        }
        std::string                  nodeName    = node["name"].as<std::string>();
        SpriteSheet&                 spriteSheet = getOrAddSpriteSheet(node["sprite"].as<std::string>().c_str());
        std::shared_ptr<PetJumpNode> p_node      = std::make_shared<PetJumpNode>(
            spriteAnimator, spriteSheet, node["framerate"].as<int>(), node["direction"].as<Vec2>(),
            node["verticalThrust"].as<float>(), node["horizontalThrust"].as<float>());
        nodes.emplace(nodeName, std::static_pointer_cast<StateMachine::Node>(p_node));
        return true;
    }

    template <typename T>
    bool AddBasicTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 2)
        {
            warning("YAML error: transition invalid");
            return false;
        }
        std::string nodeFromName = node["from"].as<std::string>();
        std::string nodeToName   = node["to"].as<std::string>();

        std::shared_ptr<T> transition = std::make_shared<T>();
        transition->to                = nodes[nodeToName];
        nodes[nodeFromName]->AddTransition(std::static_pointer_cast<StateMachine::Node::Transition>(transition));
        return true;
    }

    bool AddRandomDelayTransition(YAML::Node node, std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 4)
        {
            warning("YAML error: transition invalid");
            return false;
        }
        std::string nodeFromName = node["from"].as<std::string>();
        std::string nodeToName   = node["to"].as<std::string>();
        int         duration     = node["duration"].as<int>();
        int         interval     = node["interval"].as<int>();

        std::shared_ptr<RandomDelayTransition> transition = std::make_shared<RandomDelayTransition>(duration, interval);
        transition->to                                    = nodes[nodeToName];
        nodes[nodeFromName]->AddTransition(std::static_pointer_cast<StateMachine::Node::Transition>(transition));
        return true;
    }

    bool AddRandomDelayTransitionMultipleExit(YAML::Node                                                  node,
                                              std::map<std::string, std::shared_ptr<StateMachine::Node>>& nodes)
    {
        if (!node.IsMap() || node.size() != 4)
        {
            warning("YAML error: transition invalid");
            return false;
        }
        const std::string nodeFromName = node["from"].as<std::string>();
        const int         duration     = node["duration"].as<int>();
        const int         interval     = node["interval"].as<int>();

        std::vector<RandomDelayTransitionMultipleExit::NodeChanceToEnter> entries;

        YAML::Node toNodes = node["chanceToEnterEntries"];
        if (!toNodes || toNodes.size() == 0)
        {
            warning("YAML error: missing entires");
            return false;
        }

        for (YAML::const_iterator it = toNodes.begin(); it != toNodes.end(); ++it)
        {
            const std::string  nodeToName = it->second["to"].as<std::string>();
            const unsigned int chance     = it->second["chance"].as<unsigned int>();
            entries.emplace_back(RandomDelayTransitionMultipleExit::NodeChanceToEnter{nodes[nodeToName], chance});
        }

        std::shared_ptr<RandomDelayTransitionMultipleExit> transition =
            std::make_shared<RandomDelayTransitionMultipleExit>(duration, interval, entries);
        nodes[nodeFromName]->AddTransition(std::static_pointer_cast<StateMachine::Node::Transition>(transition));
        return true;
    }

    void update(double deltaTime)
    {
        animator.update(deltaTime);
    }

    void draw()
    {
        spriteAnimator.draw(datas, *datas.pSpriteSheetShader, datas.side);
        datas.pUnitFullScreenQuad->use();
        datas.pUnitFullScreenQuad->draw();
    }
};

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

        datas.monitors    = glfwGetMonitors(&datas.monitorCount);
        datas.videoMode   = glfwGetVideoMode(datas.monitors[0]);
        datas.petPosLimit = {datas.videoMode->width, datas.videoMode->height};
        datas.windowSize  = {1, 1};

        datas.window = glfwCreateWindow(datas.windowSize.x, datas.windowSize.y, "PetForDesktop", NULL, NULL);
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
        glGetString == nullptr;

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

    void initDrawContext()
    {
        Framebuffer::bindScreen();
        glViewport(0, 0, datas.windowSize.x, datas.windowSize.y);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);

        glClear(GL_COLOR_BUFFER_BIT);
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

            physicSystem.update(deltaTime);

            // poll for and process events
            glfwPollEvents();
        }};
        const std::function<void(double)> limitedUpdate{[&](double deltaTime) {
            // render
            initDrawContext();

            pet.draw();

            // swap front and back buffers
            glfwSwapBuffers(datas.window);
        }};

        const std::function<void(double)> limitedUpdateDebugCollision{[&](double deltaTime) {
            // fullscreen
            datas.windowSize.x = datas.videoMode->width / 1.f;
            datas.windowSize.y = datas.videoMode->height / 1.f;
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

        mainLoop.emplaceTimer(
            [&]() {
                physicSystem.update(1.f / datas.physicFrameRate);

                pet.update(1.f / datas.physicFrameRate);
            },
            1.f / datas.physicFrameRate, true);

        mainLoop.start();
        while (!glfwWindowShouldClose(datas.window))
        {
            mainLoop.update(datas.debugEdgeDetection ? unlimitedUpdateDebugCollision : unlimitedUpdate,
                            datas.debugEdgeDetection ? limitedUpdateDebugCollision : limitedUpdate);
        }
    }
};

// Disable usage of external GPU
#ifdef __cplusplus
extern "C"
{
#endif
    __declspec(dllexport) DWORD NvOptimusEnablement                = 0;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0;

#ifdef __cplusplus
}
#endif

// Disable console
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char** argv)
{
    Game game;
    game.run();

    return 0;
}