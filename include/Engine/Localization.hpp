#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#elif __APPLE__
#include <langinfo.h>
#include <xlocale.h>
#elif __linux__
#include <locale.h>
#endif

std::string getUserLocalization()
{
    std::string localization;

#ifdef _WIN32
    // Windows-specific code
    //WCHAR  langIDStr[LOCALE_NAME_MAX_LENGTH];
    char   langIDStr[LOCALE_NAME_MAX_LENGTH];
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