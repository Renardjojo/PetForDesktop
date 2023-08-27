#pragma once

#include "Engine/Singleton.hpp"
#include "Game/GameData.hpp"

class Setting : public Singleton<Setting>
{
public:
    void importFile(const char* src, GameData& data);

    void exportFile(const char* dest, GameData& data);
};