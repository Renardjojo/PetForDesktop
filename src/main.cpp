#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>
#include <stdio.h>
#include <string>

#include "INIReader.h"
#include "Vector2.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GameData
{
    // Window and monitor
    GLFWwindow*        window       = nullptr;
    GLFWmonitor**      monitors     = nullptr;
    const GLFWvidmode* videoMode    = nullptr;
    int                monitorCount = 0, windowWidth = 0, windowHeight = 0, monitorX = 0, monitorY = 0;
    vec2               windowPos = {0.f, 0.f};

    // Inputs
    float prevCursorPosX  = 0;
    float prevCursorPosY  = 0;
    float deltaCursorPosX = 0;
    float deltaCursorPosY = 0;
    int   leftButtonEvent = 0;

    // Settings
    int FPS   = 0;
    int scale = 0;

    // Physic
    vec2 velocity         = {0.f, 0.f};
    vec2 continusVelocity = {
        0.f,
        0.f}; // This value is not changed by the physic system. Usefull for movement. Friction is applied to this value
    vec2  gravity    = {0.f, 0.f};
    float bounciness = 0.f;
    float friction   = 0.f;

    // Animation
    int   animationFrameRate   = 10;
    float walkSpeed            = 0.f;
    int   walkDuration         = 1000;
    int   walkDurationInterval = 500;
    int   idleDuration         = 1000;
    int   idleDurationInterval = 500;
    bool  side; // false left / true right

    // Time
    double timeAcc = 0.f;

    // Debug
    bool showWindow;
    bool showFrameBufferBackground;
};

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
            break;
        case GLFW_RELEASE:
            datas.velocity = vec2{datas.deltaCursorPosX / datas.FPS, datas.deltaCursorPosY / datas.FPS};
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void processInput(GLFWwindow* window)
{
    GameData& datas = *static_cast<GameData*>(glfwGetWindowUserPointer(window));

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (datas.leftButtonEvent == GLFW_PRESS)
    {
        datas.windowPos += vec2{datas.deltaCursorPosX, datas.deltaCursorPosY};
        datas.deltaCursorPosX = 0;
        datas.deltaCursorPosY = 0;
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int randNum(int min, int max)
{
    return (rand() % (((max) + 1) - (min))) + (min);
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
            printf("The file '%s' was not opened\n", filename);
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
    // ------------------------------------------------------------------------
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
                printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n "
                       "-------------------------------------------------------",
                       type, infoLog);
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n "
                       "-------------------------------------------------------",
                       type, infoLog);
            }
        }
    }
};

class ScreenSpaceQuad
{
protected:
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;

public:
    ScreenSpaceQuad()
    {
        float vertices[] = {
            // positions        // texture coords
            1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top right
            1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
            -1.0f, 1.0f,  0.0f, 0.0f, 1.0f  // top left
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

class Texture
{
protected:
    unsigned int ID;
    int          width, height, nrChannels;

public:
    Texture(const char* srcPath)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // load image, create texture and generate mipmaps
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char* data = stbi_load(srcPath, &width, &height, &nrChannels, 0);
        if (data)
        {
            if (nrChannels == 4)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            puts("Failed to load texture");
        }
        stbi_image_free(data);
    }

    ~Texture()
    {
        glDeleteTextures(1, &ID);
    }

    void use()
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    int getHeight()
    {
        return height;
    }

    int getWidth()
    {
        return width;
    }
};

class SpriteSheet : public Texture
{
protected:
    int tileCount;

public:
    SpriteSheet(const char* srcPath) : Texture(srcPath)
    {
        tileCount = width / height;
    }

    void useSection(GameData& data, Shader shader, int idSection, bool hFlip = false)
    {
        data.windowWidth  = height * data.scale;
        data.windowHeight = height * data.scale;
        glfwSetWindowSize(data.window, data.windowWidth, data.windowHeight);

        float hScale  = 1.f / tileCount;
        float vScale  = 1.f; // This field can be used
        float hOffSet = idSection / (float)tileCount;
        float vOffset = 0.f; // This field can be used

        if (hFlip)
        {
            hOffSet += hScale;
            hScale *= -1;
        }

        shader.setVec4("uScaleOffSet", hScale, vScale, hOffSet, vOffset);
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
        INIReader reader(path);

        if (reader.ParseError() == -1)
        {
            printf("Could not find setting file here: %s", path);
            exit(-1);
        }

        std::string gameSettingSection = "Game setting";

        data.FPS   = std::max(reader.GetInteger(gameSettingSection, "FPS", 60), 1l);
        data.scale = std::max(reader.GetInteger(gameSettingSection, "Scale", 1), 1l);

        std::string physicSettingSection = "Physic";

        data.bounciness = std::clamp(reader.GetReal(physicSettingSection, "Bounciness", 0.1), 0.0, 1.0);
        data.gravity    = vec2{(float)reader.GetReal(physicSettingSection, "GravityX", 0.0),
                            (float)reader.GetReal(physicSettingSection, "GravityY", 9.81)};
        data.friction   = std::clamp(reader.GetReal(physicSettingSection, "Friction", 0.5), 0.0, 1.0);

        std::string animationSection = "Animation";

        data.animationFrameRate = std::max(reader.GetInteger(animationSection, "AnimationFrameRate", 1), 1l);
        data.walkSpeed          = std::max(reader.GetReal(animationSection, "WalkSpeed", 1.0), 0.0);

        data.walkDuration         = std::max(reader.GetInteger(animationSection, "WalkDuration", 1000), 0l);
        data.walkDurationInterval = reader.GetInteger(animationSection, "WalkDurationInterval", 500);
        data.idleDuration         = std::max(reader.GetInteger(animationSection, "IdleDuration", 1000), 0l);
        data.idleDurationInterval = reader.GetInteger(animationSection, "IdleDurationInterval", 500);

        std::string debugSection = "Debug";

        data.showWindow                = reader.GetBoolean(debugSection, "ShowWindow", true);
        data.showFrameBufferBackground = reader.GetBoolean(debugSection, "ShowFrameBufferBackground", true);
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

    void computeMonitorCollisions()
    {
        if (data.windowPos.x < 0.f)
        {
            data.windowPos.x = 0.f;
            data.velocity    = data.velocity.reflect(vec2::right()) * data.bounciness;
        }

        if (data.windowPos.y < 0.f)
        {
            data.windowPos.y = 0.f;
            data.velocity    = data.velocity.reflect(vec2::down()) * data.bounciness;
        }

        float maxWinPosX = data.videoMode->width - data.windowWidth;
        float maxWinPosY = data.videoMode->height - data.windowHeight;

        if (data.windowPos.x > maxWinPosX)
        {
            data.windowPos.x = maxWinPosX;
            data.velocity    = data.velocity.reflect(vec2::left()) * data.bounciness;
        }

        if (data.windowPos.y > maxWinPosY)
        {
            data.windowPos.y = maxWinPosY;
            data.velocity    = data.velocity.reflect(vec2::up()) * data.bounciness;
        }
    }

    void update(double deltaTime)
    {
        // Apply gravity if not selected
        if (data.leftButtonEvent != GLFW_PRESS)
        {
            // Acc = Sum of force / Mass
            // G is already an acceleration
            vec2 acc = data.gravity;

            // V = Acc * Time
            data.velocity += acc * deltaTime;

            // Evaluate pixel distance based on dpi and monitor size
            int width_mm, height_mm;
            glfwGetMonitorPhysicalSize(data.monitors[0], &width_mm, &height_mm);

            vec2 pixelPerMeter{(float)data.videoMode->width / (width_mm * 0.001f),
                               (float)data.videoMode->height / (height_mm * 0.001f)};

            // Pos = PrevPos + V * Time
            data.windowPos +=
                (data.continusVelocity + data.velocity) * (1.f - data.friction) * pixelPerMeter * deltaTime;

            // Apply monitor collision
            computeMonitorCollisions();
        }

        glfwSetWindowPos(data.window, data.windowPos.x, data.windowPos.y);
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

public:
    TimeManager(GameData& data) : m_fixedDeltaTime{1. / data.FPS}, datas{data}
    {
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
                assert(pNodeTransition->to != nullptr);

                pCurrentNode->onExit(blackBoard);
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
};

class PetWalkNode : public AnimationNode
{
    vec2  baseDir = {0.f, 0.f};
    float thrust  = 0.f;

public:
    PetWalkNode(SpriteAnimator& inSpriteAnimator, SpriteSheet& inSpriteSheets, int inFrameRate,
                                 vec2 inDir, float inThrust, bool inLoop = true)
        : AnimationNode(inSpriteAnimator, inSpriteSheets, inFrameRate, inLoop), baseDir{inDir}, thrust{inThrust}
    {
    }

    void onEnter(GameData& blackBoard) override
    {
        StateMachine::Node::onEnter(blackBoard);
        blackBoard.side = randNum(0, 1);
        spriteAnimator.play(spriteSheets, loop, frameRate);
        blackBoard.continusVelocity += baseDir * (blackBoard.side * 2 - 1) * thrust;
    }

    void onUpdate(GameData& blackBoard, double dt) override
    {
        StateMachine::Node::onUpdate(blackBoard, dt);
        spriteAnimator.update(dt);
    }

    void onExit(GameData& blackBoard) override
    {
        StateMachine::Node::onExit(blackBoard);
        blackBoard.continusVelocity -= baseDir * (blackBoard.side * 2 - 1) * thrust;
    }
};

struct RandomDelayTransition : public StateMachine::Node::Transition
{
protected:
    float delay = 0.f;
    float timer = 0.f;

    int baseDelay_ms = 0;
    int randomMin_ms = 0;
    int randomMax_ms = 0;

public:
    RandomDelayTransition(int inBaseDelay_ms, int inRandomMin_ms, int inRsandomMax_ms)
        : baseDelay_ms{inBaseDelay_ms}, randomMin_ms{inRandomMin_ms}, randomMax_ms{inRsandomMax_ms}
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
        delay = baseDelay_ms + randNum(randomMin_ms, randomMax_ms);
        delay *= 0.001; // to seconde
    }

    void onUpdate(GameData& blackBoard, double dt) final
    {
        timer += dt;
    }
};

struct StartLeftClicTransition : public StateMachine::Node::Transition
{
protected:
    bool leftWasPressed = false;

public:
    bool canTransition(GameData& blackBoard) final
    {
        if (blackBoard.leftButtonEvent == GLFW_PRESS)
        {
            if (leftWasPressed)
            {
                leftWasPressed = false;
                return true;
            }
            leftWasPressed = true;
        }
        return false;
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
    enum class EBehaviour : int
    {
        idle  = 0,
        idle1 = 1,
        walk  = 2,
        drag  = 3
    };

    enum class ESide
    {
        left,
        right
    };

    std::vector<SpriteSheet> spriteSheets;
    EBehaviour               state{EBehaviour::idle};
    ESide                    side{ESide::left};

    ScreenSpaceQuad screenSpaceQuad;
    Shader          shader;
    GameData&       datas;

    StateMachine   animator;
    SpriteAnimator spriteAnimator;
    int            indexCurrentAnimSprite = 0;
    bool           loopCurrentAnim;

    bool leftWasPressed = false;

public:
    Pet(GameData& data)
        : shader("./resources/shader/spriteSheet.vs", "./resources/shader/image.fs"), datas{data}, animator{data}
    {
        spriteSheets.reserve(4);
        spriteSheets.emplace_back("./resources/sprites/idle.png");
        spriteSheets.emplace_back("./resources/sprites/idle2.png");
        spriteSheets.emplace_back("./resources/sprites/walk.png");
        spriteSheets.emplace_back("./resources/sprites/drag2.png");

        // Assuming pet is alone on window
        shader.use();
        shader.setInt("uTexture", 0);
        screenSpaceQuad.use();

        createAnimationGraph();
    }

    void createAnimationGraph()
    {
        // Init all nodes
        std::shared_ptr<AnimationNode> idleNode =
            std::make_shared<AnimationNode>(spriteAnimator, spriteSheets[0], datas.animationFrameRate, true);

        std::shared_ptr<AnimationNode> grabNode =
            std::make_shared<AnimationNode>(spriteAnimator, spriteSheets[3], datas.animationFrameRate, false);

        std::shared_ptr<PetWalkNode> walkNode = std::make_shared<PetWalkNode>(
            spriteAnimator, spriteSheets[2], datas.animationFrameRate, vec2::right(), datas.walkSpeed, true);

        // Create all transitions
        // Idle to grab
        {
            std::shared_ptr<StartLeftClicTransition> transition = std::make_shared<StartLeftClicTransition>();
            transition->to                                      = grabNode;
            idleNode->transitions.emplace_back(transition);
        }

        // Create all transitions
        // Idle to walk
        {
            std::shared_ptr<RandomDelayTransition> transition = std::make_shared<RandomDelayTransition>(
                datas.idleDuration, -datas.idleDurationInterval, datas.idleDurationInterval);
            transition->to = walkNode;
            idleNode->transitions.emplace_back(transition);
        }

        // Create all transitions
        // walk to grab
        {
            std::shared_ptr<StartLeftClicTransition> transition = std::make_shared<StartLeftClicTransition>();
            transition->to                                      = grabNode;
            walkNode->transitions.emplace_back(transition);
        }

        // Create all transitions
        // walk to idle
        {
            std::shared_ptr<RandomDelayTransition> transition = std::make_shared<RandomDelayTransition>(
                datas.walkDuration, -datas.walkDurationInterval, datas.walkDurationInterval);
            transition->to = idleNode;
            walkNode->transitions.emplace_back(transition);
        }

        // Grab to idle
        {
            std::shared_ptr<EndLeftClicTransition> transition = std::make_shared<EndLeftClicTransition>();
            transition->to                                    = idleNode;
            grabNode->transitions.emplace_back(transition);
        }

        // Start state machine
        animator.init(std::static_pointer_cast<StateMachine::Node>(idleNode));
    }

    void update(double deltaTime)
    {
        animator.update(deltaTime);
    }

    void draw()
    {
        spriteAnimator.draw(datas, shader, datas.side);
        screenSpaceQuad.draw();
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
            exit(-1);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, !datas.showFrameBufferBackground);
        glfwWindowHint(GLFW_VISIBLE, datas.showFrameBufferBackground);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

        datas.monitors  = glfwGetMonitors(&datas.monitorCount);
        datas.videoMode = glfwGetVideoMode(datas.monitors[0]);

        datas.window = glfwCreateWindow(1, 1, "PetDesktop", NULL, NULL);
        if (!datas.window)
        {
            glfwTerminate();
            exit(-1);
        }

        glfwMakeContextCurrent(datas.window);

        glfwSetWindowAttrib(datas.window, GLFW_DECORATED, datas.showWindow);
        glfwSetWindowAttrib(datas.window, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        glfwDefaultWindowHints();
    }

    void initOpenGL()
    {
        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            puts("Failed to initialize GLAD");
            exit(-1);
        }
    }

public:
    Game() : setting("./resources/setting/setting.ini", datas), mainLoop(datas), physicSystem(datas)
    {
        initWindow();
        initOpenGL();

        glfwGetMonitorPos(datas.monitors[0], &datas.monitorX, &datas.monitorY);

        datas.windowPos =
            vec2{datas.monitorX + (datas.videoMode->width) / 2.f, datas.monitorY + (datas.videoMode->height) / 2.f};

        glfwSetWindowPos(datas.window, datas.windowPos.x, datas.windowPos.y);

        glfwShowWindow(datas.window);

        glfwSetWindowUserPointer(datas.window, &datas);

        glfwSetMouseButtonCallback(datas.window, mousButtonCallBack);
        glfwSetCursorPosCallback(datas.window, cursorPositionCallback);
        glfwSetFramebufferSizeCallback(datas.window, framebufferSizeCallback);


        srand(time(NULL));
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

            physicSystem.update(deltaTime);
            pet.update(deltaTime);

            // poll for and process events
            glfwPollEvents();
        }};
        const std::function<void(double)> limitedUpdate{[&](double deltaTime) {
            // render
            glClear(GL_COLOR_BUFFER_BIT);

            pet.draw();

            // swap front and back buffers
            glfwSwapBuffers(datas.window);
        }};

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);

        while (!glfwWindowShouldClose(datas.window))
        {
            mainLoop.update(unlimitedUpdate, limitedUpdate);
        }
    }
};

int main(int argc, char** argv)
{
    Game game;
    game.run();

    return 0;
}