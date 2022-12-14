#pragma once

#include "boxer/boxer.h"

#include <stdio.h> 
#include <cstdarg>
#include <string>

void log(const char* buffer)
{
#if _DEBUG
    // log into console
    fputs(buffer, stderr);
#endif
}

void logf(char const* const format, ...)
{
    va_list arglist;
    va_start(arglist, format);
#if _DEBUG
    // log into console
    vfprintf(stderr, format, arglist);
#endif
    va_end(arglist);
}

void errorAndExit(const std::string& msg)
{
    boxer::Selection selection =
        boxer::show(msg.c_str(), PROJECT_NAME " error", boxer::Style::Error, boxer::Buttons::OK);
    exit(-1);
}

void warning(const std::string& msg)
{
    boxer::show(msg.c_str(), PROJECT_NAME " warning", boxer::Style::Warning, boxer::Buttons::OK);
    exit(-1);
}