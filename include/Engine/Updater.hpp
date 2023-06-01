#pragma once

#include "Engine/Log.hpp"

#include "boxer/boxer.h"

#include <cpr/cpr.h>

#include <regex>
#include <thread>

class Updater
{
    std::thread m_thread;

public:
    Updater()
#if _NDEBUG // Do not run the update program in debug mode to avoid spamming web requests
        : m_thread(Updater::checkForUpdate)
#endif
    {
    }

    ~Updater()
    {
        m_thread.join();
    }

private:
    static void startup(LPCTSTR lpApplicationName)
    {
        // additional information
        STARTUPINFO         si;
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // start the program up
        CreateProcess(lpApplicationName, // the path
                      NULL,              // Command line
                      NULL,              // Process handle not inheritable
                      NULL,              // Thread handle not inheritable
                      FALSE,             // Set handle inheritance to FALSE
                      0,                 // No creation flags
                      NULL,              // Use parent's environment block
                      NULL,              // Use parent's starting directory
                      &si,               // Pointer to STARTUPINFO structure
                      &pi                // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );
        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    static void changeFileExtension(char* file_name)
    {
        // Find the last '.' character in the file name
        char* dot = strrchr(file_name, '.');
        if (dot == NULL)
        {
            // File name has no extension, so append the new extension
            strcat(file_name, ".exe");
        }
        else
        {
            // File name has an extension, so copy the file name up to the '.' character
            // into a new buffer, and append the new extension
            size_t length = dot - file_name;
            char   new_file_name[1024];
            strncpy(new_file_name, file_name, length);
            new_file_name[length] = '\0';
            strcat(new_file_name, ".exe");
            strncpy(file_name, new_file_name, length + 5);
        }
    }

    static void generateAndLoadFile(const char* data, size_t count)
    {
        // Generate a unique file name
        char temp_file_name[1024] = {0};
        if (tmpnam(temp_file_name) == NULL)
            return;

        // Create the temporary file
        changeFileExtension(temp_file_name);
        FILE* temp_file = fopen(temp_file_name, "w+b");
        if (temp_file == NULL)
            return;

        fwrite(data, sizeof(char), count, temp_file);
        fclose(temp_file);

        startup(temp_file_name);

        unlink(temp_file_name);
        exit(1);
    }

    static void checkForUpdate()
    {
        cpr::Response response =
            cpr::Get(cpr::Url{"https://api.github.com/repos/Renardjojo/PetDesktop/releases/latest"});
        if (response.error)
        {
            log((response.error.message + "\n").c_str());
            return;
        }

        std::string json = response.text;
        std::regex  pattern("\"tag_name\":\\s*\"(.*?)\"");
        std::smatch matches;

        if (std::regex_search(json, matches, pattern))
        {
            if (matches[1] != "v" PROJECT_VERSION)
            {
                pattern = "\"body\":\\s*\"(.*?)\"";
                std::string content =
                    std::string(PROJECT_NAME " ") + matches[1].str() + " is available. Do you want download it ?";
                // Looking for changelog
                if (std::regex_search(json, matches, pattern))
                {
                    content += "\n\n";
                    content += markdownToPlainText(matches[1].str());
                }

                boxer::Selection selection = boxer::show(content.c_str(), PROJECT_NAME " " PROJECT_VERSION " updater",
                                                         boxer::Style::Question, boxer::Buttons::YesNo);

                // TODO: Dialog pop up ?
                if (selection == boxer::Selection::Yes)
                {
                    pattern = "\"browser_download_url\":\\s*\"(.*?)\"";
                    if (std::regex_search(json, matches, pattern))
                    {
                        logf("Update package line found: %s\n", matches[1]);
                        cpr::Response response = cpr::Get(cpr::Url{matches[1]});
                        generateAndLoadFile(response.text.c_str(), response.text.size());
                    }
                    else
                    {
                        log("Update package not found\n");
                    }
                }
            }
            else
            {
                logf("The version %s is the latest\n", PROJECT_VERSION);
            }
        }
    }

    static std::string markdownToPlainText(const std::string& markdown)
    {
        // Replace '\\r' and '\\n' with '\n'
        std::string lineBreakSearch  = "\\\\n|\\\\r";
        std::string lineBreakReplace = "\n";
        std::regex  lineBreakRegex(lineBreakSearch);
        std::string plainText = std::regex_replace(markdown, lineBreakRegex, lineBreakReplace);

        // Regular expressions for Markdown syntax
        std::regex headerRegex("#+\\s*(.*)");
        std::regex listRegex("\\*\\s+(.*)");
        std::regex linkRegex("\\[(.*?)\\]\\((https?://\\S+)\\)");
        std::regex boldRegex("\\*\\*(.*?)\\*\\*");

        // Replace headers with plain text
        plainText = std::regex_replace(plainText, headerRegex, "$1");

        // Replace list items with plain text
        plainText = std::regex_replace(plainText, listRegex, "- $1");

        // Replace links with plain text
        plainText = std::regex_replace(plainText, linkRegex, "$2");

        // Remove bold sections
        plainText = std::regex_replace(plainText, boldRegex, "$1");

        return plainText;
    }
};