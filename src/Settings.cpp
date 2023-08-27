#include "Engine/Settings.hpp"
#include "Engine/Log.hpp"

#include "yaml-cpp/yaml.h"

#include <algorithm>

void Setting::importFile(const char* src, GameData& data)
{
    YAML::Node animGraph = YAML::LoadFile(src);
    if (!animGraph)
    {
        errorAndExit(std::string("Could not find setting file here: ") + src);
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
        data.continuousCollisionMaxSqrVelocity =
            std::max(nodesSection["ContinuousCollisionMaxVelocity"].as<float>(), 0.f);
        data.continuousCollisionMaxSqrVelocity *= data.continuousCollisionMaxSqrVelocity;
        data.footBasementWidth  = std::max(nodesSection["FootBasementWidth"].as<int>(), 2);
        data.footBasementHeight = std::max(nodesSection["FootBasementHeight"].as<int>(), 2);
        data.collisionPixelRatioStopMovement =
            std::clamp(nodesSection["CollisionPixelRatioStopMovement"].as<float>(), 0.f, 1.f);
        data.isGroundedDetection = std::max(nodesSection["IsGroundedDetection"].as<float>(), 0.f);
        data.releaseImpulse      = std::max(nodesSection["InputReleaseImpulse"].as<float>(), 0.f);
    }

    {
        section                 = "GamePlay";
        YAML::Node nodesSection = animGraph[section];
        if (!nodesSection)
            errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

        data.coyoteTimeCursorPos = std::max(nodesSection["CoyoteTimeCursorMovement"].as<float>(), 0.f);
    }

    {
        section                 = "Window";
        YAML::Node nodesSection = animGraph[section];
        if (!nodesSection)
            errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

        data.fullScreenWindow          = nodesSection["FullScreenWindow"].as<bool>();
        data.showWindow                = nodesSection["ShowWindow"].as<bool>();
        data.showFrameBufferBackground = nodesSection["ShowFrameBufferBackground"].as<bool>();
        data.useForwardWindow          = nodesSection["UseForwardWindow"].as<bool>();
        data.useMousePassThoughWindow  = nodesSection["UseMousePassThoughWindow"].as<bool>();
    }

    {
        section                 = "Window";
        YAML::Node nodesSection = animGraph[section];
        if (!nodesSection)
            errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

        data.showWindow                = nodesSection["ShowWindow"].as<bool>();
        data.showFrameBufferBackground = nodesSection["ShowFrameBufferBackground"].as<bool>();
        data.useForwardWindow          = nodesSection["UseForwardWindow"].as<bool>();
        data.useMousePassThoughWindow  = nodesSection["UseMousePassThoughWindow"].as<bool>();
    }

    {
        section                 = "Style";
        YAML::Node nodesSection = animGraph[section];
        if (!nodesSection)
            errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

        data.styleName = nodesSection["Theme"].as<std::string>();
    }

    {
        section                 = "Debug";
        YAML::Node nodesSection = animGraph[section];
        if (!nodesSection)
            errorAndExit("Cannot find \"" + section + "\" in setting.yaml");

        data.debugEdgeDetection = nodesSection["ShowEdgeDetection"].as<bool>();
    }
}

void Setting::exportFile(const char* dest, GameData& data)
{
    FILE* file = nullptr;
    if (fopen_s(&file, dest, "wt"))
    {
        logf("The file \"%s\" was not opened to write\n", dest);
        return;
    }

    YAML::Emitter out;

    std::string section;
    {
        section = "Game";
        out << YAML::BeginMap;
        out << YAML::Block << section;
        out << YAML::BeginMap;
        out << YAML::Key << "FPS" << YAML::Value << data.FPS;
        out << YAML::Key << "Scale" << YAML::Value << data.scale;
        out << YAML::Key << "RandomSeed" << YAML::Value << data.randomSeed;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "Physic";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "PhysicFrameRate" << YAML::Value << data.physicFrameRate;
        out << YAML::Key << "Bounciness" << YAML::Value << YAML::Precision(4) << data.bounciness;
        out << YAML::Key << "GravityX" << YAML::Value << YAML::Precision(4) << data.gravity.x;
        out << YAML::Key << "GravityY" << YAML::Value << YAML::Precision(4) << data.gravity.y;
        out << YAML::Key << "Friction" << YAML::Value << YAML::Precision(4) << data.friction;
        out << YAML::Key << "ContinuousCollisionMaxVelocity" << YAML::Value << std::sqrt(data.continuousCollisionMaxSqrVelocity);
        out << YAML::Key << "FootBasementWidth" << YAML::Value << data.footBasementWidth;
        out << YAML::Key << "FootBasementHeight" << YAML::Value << data.footBasementHeight;
        out << YAML::Key << "CollisionPixelRatioStopMovement" << YAML::Value << YAML::Precision(4)
             << data.collisionPixelRatioStopMovement;
        out << YAML::Key << "IsGroundedDetection" << YAML::Value << data.isGroundedDetection;
        out << YAML::Key << "InputReleaseImpulse" << YAML::Value << data.releaseImpulse;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "GamePlay";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "CoyoteTimeCursorMovement" << YAML::Value << YAML::Precision(4) << data.coyoteTimeCursorPos;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "Window";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "FullScreenWindow" << YAML::Value << data.fullScreenWindow;
        out << YAML::Key << "ShowWindow" << YAML::Value << data.showWindow;
        out << YAML::Key << "ShowFrameBufferBackground" << YAML::Value << data.showFrameBufferBackground;
        out << YAML::Key << "UseForwardWindow" << YAML::Value << data.useForwardWindow;
        out << YAML::Key << "UseMousePassThoughWindow" << YAML::Value << data.useMousePassThoughWindow;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "Style";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "Theme" << YAML::Value << data.styleName;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "Accessibility";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "GlobalScale" << YAML::Value << 2;
        out << YAML::Key << "FontScale" << YAML::Value << 14;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    {
        section = "Debug";
        out << YAML::BeginMap;
        out << section;
        out << YAML::BeginMap;
        out << YAML::Key << "ShowEdgeDetection" << YAML::Value << data.debugEdgeDetection;
        out << YAML::EndMap;
        out << YAML::EndMap;
    }

    fwrite(out.c_str(), sizeof(char), out.size(), file);
    fclose(file);
}