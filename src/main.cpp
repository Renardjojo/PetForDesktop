#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cmath>
#include <functional>
#include <queue>
#include <stdio.h>
#include <string>

#include "INIReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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

    void useSection(GLFWwindow* win, Shader shader, int idSection, int scale)
    {
        glfwSetWindowSize(win, height * scale, height * scale);
        shader.setVec4("uScaleOffSet", 1.f / tileCount, 1.f, idSection / (float)tileCount, 0.f);
        use();
    }

    int getTileCount() const
    {
        return tileCount;
    }
};

class Setting
{
protected:
    int FPS;
    int scale;
        
public:
    Setting(const char* path)
    {
        INIReader reader(path);

        if (reader.ParseError() == -1)
        {
            printf("Could not find setting file here: %s", path);
            exit(-1);
        }

        FPS = reader.GetInteger("Game setting", "FPS", 60);
        scale = reader.GetInteger("Game setting", "Scale", 1);
    }

    int getFPS() const
    {
        return FPS;
    }

    
    int getScale() const
    {
        return scale;
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

    double m_timeAccLoop    = 0.;
    double m_deltaTime      = 0.;
    double m_timeAcc        = 0.f;
    double m_fixedDeltaTime = 1. / 60.;

    std::priority_queue<TimerTask, std::vector<TimerTask>, std::greater<TimerTask>> m_unscaledTimerQueue;

public:
    TimeManager(int frameRate) : m_fixedDeltaTime{1. / frameRate}
    {
    }

    double getTimeAcc() const
    {
        return m_timeAcc;
    }

    void emplaceTimerEvent(std::function<void()> eventFunct, double delay, bool isLooping = false)
    {
        m_unscaledTimerQueue.emplace(eventFunct, delay, delay + m_timeAcc, isLooping);
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
        m_timeAcc += m_deltaTime;
        m_timeAcc *= !isinf(m_timeAcc); // reset if isInf (avoid conditionnal jump)

        /*Fixed update*/
        m_timeAccLoop += m_deltaTime;

        while (m_timeAccLoop >= m_fixedDeltaTime)
        {
            limitedUpdateFunction(m_deltaTime);
            m_timeAccLoop -= m_fixedDeltaTime;
        }

        /*Update timer queue task*/
        while (!m_unscaledTimerQueue.empty() && m_unscaledTimerQueue.top().globalTimer <= m_timeAcc)
        {
            const TimerTask& timerTask = m_unscaledTimerQueue.top();
            timerTask.task();

            if (timerTask.isLooping)
            {
                emplaceTimerEvent(timerTask.task, timerTask.localTimer, timerTask.isLooping);
            }
            m_unscaledTimerQueue.pop();
        }
    }
};

class Game
{
protected:
    GLFWwindow*        window      = nullptr;
    GLFWmonitor**      monitors    = nullptr;
    const GLFWvidmode* videoMode   = nullptr;
    int                windowSizeW = 640, windowSizeH = 480;
    int                monitorCount, windowWidth, windowHeight, monitorX, monitorY;

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

        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

        monitors  = glfwGetMonitors(&monitorCount);
        videoMode = glfwGetVideoMode(monitors[0]);

        window = glfwCreateWindow(windowSizeW, windowSizeH, "PetDesktop", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            exit(-1);
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
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
    Game()
    {
        initWindow();
        initOpenGL();

        glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);

        glfwSetWindowPos(window, monitorX + (videoMode->width - windowSizeW) / 2,
                         monitorY + (videoMode->height - windowSizeH) / 2);

        glfwShowWindow(window);
    }

    ~Game()
    {
        glfwTerminate();
    }

    void run()
    {
        ScreenSpaceQuad screenSpaceQuad;
        SpriteSheet     texture("./resources/sprites/Walk.png");
        Shader          shader("./resources/shader/spriteSheet.vs", "./resources/shader/image.fs");
        Setting         setting("./resources/setting/setting.ini");
        TimeManager     mainLoop(setting.getFPS());

        const std::function<void(double)> unlimitedUpdate{[&](double deltaTime) {
            processInput(window);

            // poll for and process events
            glfwPollEvents();
        }};
        const std::function<void(double)> limitedUpdate{[&](double deltaTime) {
            // render
            glClear(GL_COLOR_BUFFER_BIT);

            // bind textures on corresponding texture units
            int index = fmod(mainLoop.getTimeAcc() * setting.getFPS(), texture.getTileCount());
            texture.useSection(window, shader, index, setting.getScale());

            screenSpaceQuad.draw();

            // swap front and back buffers
            glfwSwapBuffers(window);
        }};
        // setting.create("./resources/setting/setting.yml");

        shader.use();
        shader.setInt("uTexture", 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);

        // render container
        shader.use();
        screenSpaceQuad.use();

        while (!glfwWindowShouldClose(window))
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