#pragma once

#include "Game/GameData.hpp"
#include "Engine/Vector2.hpp"
#include "Engine/ScreenShoot.hpp"
#include "Engine/Texture.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>

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
            data.petPos.x = static_cast<float>(data.petPosLimit.x);
            data.velocity = data.velocity.reflect(Vec2::left()) * data.bounciness;
        }

        if (data.petPos.y > data.petPosLimit.y)
        {
            data.petPos.y = static_cast<float>(data.petPosLimit.y);
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
            screenShootPosX  = 0;
            screenShootPosY  = 0;
            screenShootSizeX = data.windowSize.x;
            screenShootSizeY = data.windowSize.y;
        }
        else
        {
            const float xPadding = prevToNewWinPos.x < 0.f ? prevToNewWinPos.x : 0.f;
            const float yPadding = prevToNewWinPos.y < 0.f ? prevToNewWinPos.y : 0.f;

            screenShootPosX  = static_cast<int>(data.petPos.x + data.petSize.x / 2.f + xPadding - data.footBasasementWidth / 2.f);
            screenShootPosY  = static_cast<int>(data.petPos.y + data.petSize.y + 1 + yPadding - data.footBasasementHeight / 2.f);
            screenShootSizeX = static_cast<int>(abs(prevToNewWinPos.x) + data.footBasasementWidth);
            screenShootSizeY = static_cast<int>(abs(prevToNewWinPos.y) + data.footBasasementHeight);
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
            data.edgeDetectionShaders[0].setVec2("resolution", static_cast<float>(pxlData.width), static_cast<float>(pxlData.height));
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
            data.edgeDetectionShaders[1].setVec2("resolution", static_cast<float>(pxlData.width), static_cast<float>(pxlData.height));
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
            data.velocity += acc * (float)deltaTime;

            const Vec2 prevWinPos = data.petPos;
            // Pos = PrevPos + V * Time
            const Vec2 newWinPos = data.petPos + ((data.continusVelocity + data.velocity) * (1.f - data.friction) *
                                                  data.pixelPerMeter * (float)deltaTime);
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
                if (data.isGrounded && data.petPos.y != data.petPosLimit.y)
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

        data.windowPos.x = static_cast<int>(data.petPos.x) - data.windowMinExt.x;
        data.windowPos.y = static_cast<int>(data.petPos.y) - data.windowMinExt.y;
        glfwSetWindowPos(data.window, data.windowPos.x, data.windowPos.y);
    }
};