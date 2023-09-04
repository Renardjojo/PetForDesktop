#pragma once

#include "Engine/Singleton.hpp"
#include <string>
#include <map>

class Localization : public Singleton<Localization>
{
private:
    std::map<std::string, std::string> localDatas; // Key with text associate

public:
    void init();

    void importLocalization(const char* src, const std::string& local);

    static std::string getUserLocalization();
};