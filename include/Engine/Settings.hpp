#pragma once

#include "Game/GameData.hpp"
#include "Engine/Log.hpp"

#include "yaml-cpp/yaml.h"

#include <algorithm>

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
            section                 = "Physic";
            YAML::Node nodesSection = animGraph[section];
            if (!nodesSection)
                errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

            data.physicFrameRate = std::max(nodesSection["PhysicFrameRate"].as<int>(), 0);
            data.bounciness      = std::clamp(nodesSection["Bounciness"].as<float>(), 0.f, 1.f);
            data.gravity         = Vec2{nodesSection["GravityX"].as<float>(), nodesSection["GravityY"].as<float>()};
            data.gravityDir      = data.gravity.normalized();
            data.friction        = std::clamp(nodesSection["Friction"].as<float>(), 0.f, 1.f);
            data.continusCollisionMaxSqrVelocity =
                std::max(nodesSection["ContinusCollisionMaxVelocity"].as<float>(), 0.f);
            data.continusCollisionMaxSqrVelocity *= data.continusCollisionMaxSqrVelocity;
            data.footBasasementWidth  = std::max(nodesSection["FootBasasementWidth"].as<int>(), 2);
            data.footBasasementHeight = std::max(nodesSection["FootBasasementHeight"].as<int>(), 2);
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