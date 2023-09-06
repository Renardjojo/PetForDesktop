#pragma once

#include "Engine/ClassUtility.hpp"
#include "Engine/Singleton.hpp"

#include <map>
#include <string>
#include <vector>

class Localization : public Singleton<Localization>
{
private:
    const char*                        m_filePath            = RESOURCE_PATH "/setting/localization.yaml";
    std::string                        m_currentLocalization = "en";
    std::vector<std::string>           m_availableLocalizations;
    std::map<std::string, std::string> m_localDatas; // Key with text associate

public:
    GETTER_BY_REF(AvailableLocalizations, m_availableLocalizations);
    GETTER_BY_REF(CurrentLocalization, m_currentLocalization);

    void init();

    void importLocalization(const std::string& local);

    std::string getLocal(const std::string& key) const;
    std::string getLocal(const std::string& key, const std::string& fallback) const;

    static std::string getUserLocalization();
};