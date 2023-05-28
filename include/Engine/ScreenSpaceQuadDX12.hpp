#pragma once

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

    }

    ~ScreenSpaceQuad()
    {

    }

    void use()
    {
    }

    void draw()
    {
    }
};