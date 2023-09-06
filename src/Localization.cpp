#include "Engine/Localization.hpp"

#include "Engine/log.hpp"

#ifdef _WIN32
#include <Windows.h>
#elif __APPLE__
#include <langinfo.h>
#include <xlocale.h>
#elif __linux__
#include <locale.h>
#endif

#include "yaml-cpp/yaml.h"

void Localization::init()
{
    importLocalization(getUserLocalization());
}

void Localization::importLocalization(const std::string& local)
{
    YAML::Node root = YAML::LoadFile(m_filePath);
    if (!root)
    {
        errorAndExit(std::string("Could not find file here: ") + m_filePath);
    }

    m_availableLocalizations.clear();

    YAML::Node nodesSection;
    for (auto&& nodeLanguages : root)
    {
        m_availableLocalizations.emplace_back(nodeLanguages.begin()->first.Scalar());
        nodesSection = nodeLanguages[local];
        if (nodesSection)
        {
            m_currentLocalization = local;
            m_localDatas.clear();
            for (auto&& node : nodesSection)
            {
                m_localDatas.emplace(node.first.Scalar(), node.second.as<std::string>());
            }
        }
    }
}

std::string Localization::getLocal(const std::string& key) const
{
    return getLocal(key, key);
}

std::string Localization::getLocal(const std::string& key, const std::string& fallback) const
{
    auto it = m_localDatas.find(key);
    if (it != m_localDatas.end())
    {
        return it->second;
    }
    else
    {
#if _DEBUG
        logf("Localization fallback use for key: %s\n", key);
#endif
        return fallback;
    }
}

std::string Localization::getUserLocalization()
{
    std::string localization;

#ifdef _WIN32
    // Windows-specific code
    // WCHAR  langIDStr[LOCALE_NAME_MAX_LENGTH];
    char langIDStr[LOCALE_NAME_MAX_LENGTH];
    GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME, langIDStr, LOCALE_NAME_MAX_LENGTH);
    localization = langIDStr;
#elif __APPLE__
    // macOS-specific code
    char* langIDStr = nl_langinfo(CODESET);
    localization    = langIDStr;
#elif __linux__
    // Linux-specific code
    setlocale(LC_ALL, ""); // Set the locale to the user's default
    const char* langIDStr = nl_langinfo(CODESET);
    localization          = langIDStr;
#endif

    return localization;
}