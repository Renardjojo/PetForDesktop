#include "Engine/Settings.hpp"
#include "Engine/Log.hpp"

#include "yaml-cpp/yaml.h"

#include <algorithm>

void Setting::importFile(const char* src, GameData& data)
{
    YAML::Node root = YAML::LoadFile(src);
    if (!root)
    {
        errorAndExit(std::string("Could not find setting file here: ") + src);
    }

    YAML::Node nodesSection;
    for (auto roleIter = root.begin(); roleIter != root.end(); roleIter++)
    {
        nodesSection = (*roleIter)["Game"];
        if (nodesSection)
        {
            data.FPS        = std::max(nodesSection["FPS"].as<int>(), 1);
            data.randomSeed = nodesSection["RandomSeed"].as<int>();
            continue;
        }

        nodesSection = (*roleIter)["Physic"];
        if (nodesSection)
        {
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
            data.isGroundedDetection   = std::max(nodesSection["IsGroundedDetection"].as<float>(), 0.f);
            data.releaseImpulse        = std::max(nodesSection["InputReleaseImpulse"].as<float>(), 0.f);
            data.screenCaptureInterval = std::max(nodesSection["ScreenCaptureInterval"].as<int>(), 10);
            continue;
        }

        nodesSection = (*roleIter)["GamePlay"];
        if (nodesSection)
        {
            data.coyoteTimeCursorPos = std::max(nodesSection["CoyoteTimeCursorMovement"].as<float>(), 0.f);
            continue;
        }

        nodesSection = (*roleIter)["Window"];
        if (nodesSection)
        {
            data.fullScreenWindow          = nodesSection["FullScreenWindow"].as<bool>();
            data.showWindow                = nodesSection["ShowWindow"].as<bool>();
            data.showFrameBufferBackground = nodesSection["ShowFrameBufferBackground"].as<bool>();
            data.useForwardWindow          = nodesSection["UseForwardWindow"].as<bool>();
            data.useMousePassThoughWindow  = nodesSection["UseMousePassThoughWindow"].as<bool>();
            continue;
        }

        nodesSection = (*roleIter)["Style"];
        if (nodesSection)
        {
            data.styleName = nodesSection["Theme"].as<std::string>();
            continue;
        }

        nodesSection = (*roleIter)["Accessibility"];
        if (nodesSection)
        {
            data.scale     = std::max(nodesSection["Scale"].as<int>(), 1);
            data.textScale = std::max(nodesSection["TextScale"].as<float>(), 1.f);
            continue;
        }

        nodesSection = (*roleIter)["Debug"];
        if (nodesSection)
        {
            data.debugEdgeDetection = nodesSection["ShowEdgeDetection"].as<bool>();
            continue;
        }
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
    out << YAML::BeginSeq;
    std::string section;
    {
        section = "Game";
        out << YAML::BeginMap;
        out << YAML::Block << section;
        out << YAML::BeginMap;
        out << YAML::Key << "FPS" << YAML::Value << data.FPS;
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
        out << YAML::Key << "ContinuousCollisionMaxVelocity" << YAML::Value
            << std::sqrt(data.continuousCollisionMaxSqrVelocity);
        out << YAML::Key << "FootBasementWidth" << YAML::Value << data.footBasementWidth;
        out << YAML::Key << "FootBasementHeight" << YAML::Value << data.footBasementHeight;
        out << YAML::Key << "CollisionPixelRatioStopMovement" << YAML::Value << YAML::Precision(4)
            << data.collisionPixelRatioStopMovement;
        out << YAML::Key << "IsGroundedDetection" << YAML::Value << data.isGroundedDetection;
        out << YAML::Key << "InputReleaseImpulse" << YAML::Value << data.releaseImpulse;
        out << YAML::Key << "ScreenCaptureInterval" << YAML::Value << data.screenCaptureInterval;
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
        out << YAML::Key << "Scale" << data.scale;
        out << YAML::Key << "TextScale" << data.textScale;
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
    out << YAML::EndSeq;
    fwrite(out.c_str(), sizeof(char), out.size(), file);
    fclose(file);
}