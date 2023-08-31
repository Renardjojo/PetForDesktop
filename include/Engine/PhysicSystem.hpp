#pragma once

#include "Engine/ScreenShoot.hpp"

#ifdef USE_OPENGL_API
#include "Engine/Graphics/TextureOGL.hpp"
#include "Engine/Graphics/FramebufferOGL.hpp"
#include "Engine/Graphics/ShaderOGL.hpp"
#include "Engine/Graphics/ScreenSpaceQuadOGL.hpp"
#endif // USE_OPENGL_API

#include "Engine/Vector2.hpp"
#include "Engine/PhysicComponent.hpp"
#include "Engine/InteractionComponent.hpp"
#include "Engine/Rect.hpp"
#include "Game/GameData.hpp"

#include <cmath>

class PhysicSystem
{
protected:
    GameData& data;
    ScreenCaptureLite liteCapture;

public:
    PhysicSystem(GameData& data) : data{data}
    {
    }

    bool checkIsGrounded(const PhysicComponent& comp)
    {
        float velocityLength     = comp.velocity.length();

        if (velocityLength < FLT_EPSILON)
            return true;

        float dotGravityVelocity = (- data.gravityDir).dot(comp.velocity / velocityLength);
        return dotGravityVelocity > 0.8 && velocityLength < data.isGroundedDetection;
    }

    static bool isRectDisjointRectB(const Vec2i posA, const Vec2i sizeA, const Vec2i posB, const Vec2i sizeB)
    {
        // Check if the rectangles are disjoint (i.e. do not overlap)
        return posA.x + sizeA.x < posB.x || posA.x > posB.x + sizeB.x || posA.y + sizeA.y < posB.y ||
               posA.y > posB.y + sizeB.y;
    }

    static bool isRectAInsideRectB(const Vec2i posA, const Vec2i sizeA, const Vec2i posB, const Vec2i sizeB)
    {
        return posA.x > posB.x && posA.x + sizeA.x < posB.x + sizeB.x && posA.y > posB.y && posA.y + sizeA.y < posB.y + sizeB.y;
    }

    void computeMonitorCollisions(PhysicComponent& comp)
    {
        std::vector<Vec2i> monitorsPosition;
        std::vector<Vec2i> monitorSize;
        bool               isOutside          = true;
        int                screenOverlapCount = 0;

        // 1: Check if pet is outside of all monitors
        for (int i = 0; i < data.monitors.getMonitorsCount(); ++i)
        {
            monitorsPosition.emplace_back();
            monitorSize.emplace_back();
            data.monitors.getMonitorPosition(i, monitorsPosition[i]);
            data.monitors.getMonitorSize(i, monitorSize[i]);
            bool isOutsideOfCurrentMonitor =
                isRectDisjointRectB(comp.getRect().getPosition(), comp.getRect().getSize(), monitorsPosition[i], monitorSize[i]);
            bool iInsideOfCurrentMonitor =
                isRectAInsideRectB(comp.getRect().getPosition(), comp.getRect().getSize(), monitorsPosition[i], monitorSize[i]);

            screenOverlapCount += !iInsideOfCurrentMonitor && !isOutsideOfCurrentMonitor;

            isOutside &= isOutsideOfCurrentMonitor;
        }

        // 2: If pet is outside need correction
        float minSqrDistance    = FLT_MAX;
        comp.isOnBottomOfWindow = false;
        Vec2 reelPositionCorrection;

        // Check if only one screen overlap is not perfect but cover the majority of cases
        comp.touchScreenEdge = isOutside || screenOverlapCount == 1;
        if (comp.touchScreenEdge)
        {
            for (int i = 0; i < data.monitors.getMonitorsCount(); ++i)
            {
                Vec2 positionCorrection = comp.getRect().getPosition();
                bool isOnBottom         = false;

                if (comp.getRect().getCornerMin().x <= monitorsPosition[i].x)
                {
                    positionCorrection.x = monitorsPosition[i].x;
                }
                else if (comp.getRect().getCornerMax().x >= monitorsPosition[i].x + monitorSize[i].x)
                {
                    positionCorrection.x = monitorsPosition[i].x + monitorSize[i].x - comp.getRect().getSize().x;
                }

                if (comp.getRect().getCornerMin().y <= monitorsPosition[i].y)
                {
                    positionCorrection.y = monitorsPosition[i].y;
                }
                else if (comp.getRect().getCornerMax().y >= monitorsPosition[i].y + monitorSize[i].y)
                {
                    positionCorrection.y = monitorsPosition[i].y + monitorSize[i].y - comp.getRect().getSize().y;
                    isOnBottom           = true;
                }

                float currentSqrDistance = (positionCorrection - comp.getRect().getPosition()).sqrLength();
                if (currentSqrDistance < minSqrDistance)
                {
                    comp.isOnBottomOfWindow = isOnBottom;
                    minSqrDistance          = currentSqrDistance;
                    reelPositionCorrection  = positionCorrection;
                }
            }

            if (minSqrDistance > FLT_EPSILON)
                comp.velocity =
                    comp.velocity.reflect((reelPositionCorrection - comp.getRect().getPosition()).normalized()) * data.bounciness;

            comp.isGrounded = (comp.isOnBottomOfWindow &&
                               comp.velocity.sqrLength() < data.isGroundedDetection * data.isGroundedDetection) ||
                              checkIsGrounded(comp);
            comp.velocity *= !comp.isGrounded; // reset velocity if is grounded

            comp.getRect().setPosition(reelPositionCorrection);
        }
    }

    void updateCollisionTexture(const PhysicComponent& comp, const Vec2 prevToNewWinPos)
    {
        int screenShootPosX, screenShootPosY, screenShootSizeX, screenShootSizeY;
        if (data.debugEdgeDetection)
        {
            screenShootPosX  = 0;
            screenShootPosY  = 0;
            screenShootSizeX = data.window->getSize().x;
            screenShootSizeY = data.window->getSize().y;
        }
        else
        {
            const float xPadding = prevToNewWinPos.x < 0.f ? prevToNewWinPos.x : 0.f;
            const float yPadding = prevToNewWinPos.y < 0.f ? prevToNewWinPos.y : 0.f;

            screenShootPosX  = static_cast<int>(comp.getRect().getPosition().x + comp.getRect().getSize().x / 2.f + xPadding -
                                               data.footBasementWidth / 2.f);
            screenShootPosY  = static_cast<int>(comp.getRect().getPosition().y + comp.getRect().getSize().y + 1 + yPadding -
                                               data.footBasementHeight / 2.f);
            screenShootSizeX = static_cast<int>(abs(prevToNewWinPos.x) + data.footBasementWidth);
            screenShootSizeY = static_cast<int>(abs(prevToNewWinPos.y) + data.footBasementHeight);
        }

        const ScreenCaptureLite::ImageData& pxlData =
            liteCapture.getMonitorRegion(screenShootPosX, screenShootPosY, screenShootSizeX, screenShootSizeY);

        data.pCollisionTexture     = std::make_unique<Texture>(pxlData.bits.get(), pxlData.width, pxlData.height, 4);
        data.pEdgeDetectionTexture = std::make_unique<Texture>(pxlData.width, pxlData.height, 4);

#if USE_OPENGL_API
        glDisable(GL_BLEND);
        glViewport(0, 0, pxlData.width, pxlData.height);
#endif

        if (data.edgeDetectionShaders.size() == 1)
        {
            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pEdgeDetectionTexture);

            data.edgeDetectionShaders[0]->use();
            data.edgeDetectionShaders[0]->setInt("uTexture", 0);
            data.edgeDetectionShaders[0]->setVec2("resolution", static_cast<float>(pxlData.width),
                                                 static_cast<float>(pxlData.height));
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();
        }
        else
        {
            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pCollisionTexture);

            data.edgeDetectionShaders[0]->use();
            data.edgeDetectionShaders[0]->setInt("uTexture", 0);
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();

            data.pFramebuffer->bind();
            data.pFramebuffer->attachTexture(*data.pEdgeDetectionTexture);

            data.edgeDetectionShaders[1]->use();
            data.edgeDetectionShaders[1]->setInt("uTexture", 0);
            data.edgeDetectionShaders[1]->setVec2("resolution", static_cast<float>(pxlData.width),
                                                 static_cast<float>(pxlData.height));
            data.pCollisionTexture->use();
            data.pFullScreenQuad->use();
            data.pFullScreenQuad->draw();
        }
    }

    bool processContinuousCollision(const PhysicComponent& comp, const Vec2 prevToNewWinPos, Vec2& newPos)
    {
        // Main idear is the we will take a screen shoot of the dimension of the velocity vector (depending on it's
        // magnitude)
        // Thanks to this texture, we will iterate on pixel base on velocity vector to check collision
        // Screen shoot will be post processed with edge detection alogorythm to have only white and bblack values.
        // White will be the collision

        if (prevToNewWinPos.sqrLength() == 0.f)
            return false;

        updateCollisionTexture(comp, prevToNewWinPos);

        std::vector<unsigned char> pixels;
        data.pEdgeDetectionTexture->use();
        data.pEdgeDetectionTexture->getPixels(pixels);

        int dataPerPixel = data.pEdgeDetectionTexture->getChannelsCount();

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

        float row    = prevToNewWinPosDir.y < 0.f ? height - data.footBasementHeight : 0.f;
        float column = prevToNewWinPosDir.x < 0.f ? width - data.footBasementWidth : 0.f;

        int iterationCount = iterationOnX ? width - data.footBasementWidth : height - data.footBasementHeight;
        for (int i = 0; i < iterationCount + 1; i++)
        {
            float count = 0;

            for (int y = 0; y < data.footBasementHeight; y++)
            {
                for (int x = 0; x < data.footBasementWidth; x++)
                {
                    // flip Y and find index
                    int rowFlipped = height - 1 - (int)row - y;
                    int index      = (rowFlipped * width + (int)column + x) * dataPerPixel;
                    count += pixels[index] == 255;
                }
            }
            count /= data.footBasementWidth * data.footBasementHeight;

            if (count > data.collisionPixelRatioStopMovement)
            {
                newPos = comp.getRect().getPosition() + Vec2(column, row);
                return true;
            }
            row += prevToNewWinPosDir.y;
            column += prevToNewWinPosDir.x;
        }
        return false;
    }

    bool CatpureScreenCollision(const PhysicComponent& comp, const Vec2 prevToNewWinPos, Vec2& newPos)
    {
        return processContinuousCollision(comp, prevToNewWinPos, newPos);
    }

    void update(PhysicComponent& comp, InteractionComponent& interactionComp, double deltaTime)
    {
        // Apply gravity if not selected
        if (interactionComp.isLeftSelected)
        {
            Vec2 movement = {data.deltaCursorPosX, data.deltaCursorPosY};
            comp.getRect().setPosition(comp.getRect().getPosition() + movement);

            data.deltaCursorPosX = 0;
            data.deltaCursorPosY = 0;
        }
        else
        {
            // Acc = Sum of force / Mass
            // G is already an acceleration
            const Vec2 acc = data.gravity * comp.applyGravity * !comp.isGrounded;

            // V = Acc * Time
            comp.velocity += acc * (float)deltaTime;

            const Vec2 prevWinPos = comp.getRect().getPosition();
            // Pos = PrevPos + V * Time
            const Vec2 newWinPos = comp.getRect().getPosition() + ((comp.continuousVelocity + comp.velocity) * (1.f - data.friction) *
                                                  data.pixelPerMeter * (float)deltaTime);
            
            const Vec2 prevToNewWinPos = newWinPos - prevWinPos;
            const float sqrDistMovement    = prevToNewWinPos.sqrLength();
            if ((sqrDistMovement <= data.continuousCollisionMaxSqrVelocity && prevToNewWinPos.y > 0.f) ||
                data.debugEdgeDetection)
            {
                Vec2 newPos;
                if (CatpureScreenCollision(comp, prevToNewWinPos, newPos))
                {
                    Vec2 collisionPos = newPos;
                    comp.getRect().setPosition(collisionPos);
                    comp.velocity     = comp.velocity.reflect(Vec2::up()) * data.bounciness;

                    // check if is grounded
                    comp.isGrounded = checkIsGrounded(comp);
                    comp.velocity *= !comp.isGrounded; // reset velocity if is grounded
                }
                else
                {
                    comp.getRect().setPosition(newWinPos);
                }
            }
            else
            {
                // Update is grounded
                if (comp.isGrounded && !comp.isOnBottomOfWindow)
                {
                    Vec2 newPos;
                    Vec2 footBasement((float)data.footBasementWidth, (float)data.footBasementHeight);
                    comp.isGrounded = CatpureScreenCollision(comp, footBasement, newPos);
                }

                comp.getRect().setPosition(newWinPos);
            }

            // Apply monitor collision
            if (sqrDistMovement > FLT_EPSILON)
                computeMonitorCollisions(comp);
        }
    }
};