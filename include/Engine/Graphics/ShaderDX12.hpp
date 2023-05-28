#pragma once

#include "Engine/FileReader.hpp"

#include <stdlib.h>

class Shader
{
public:
    unsigned int ID;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        logf("Parse files: %s %s\n", vertexPath, fragmentPath);
        FileReader  vertexCodeFile(vertexPath);
        FileReader  fragmentCodeFile(fragmentPath);
        const char* vShaderCode = vertexCodeFile.get();
        const char* fShaderCode = fragmentCodeFile.get();


        log("Shader compilation done\n");
    }

    void use()
    {
    }

    void setBool(const char* name, bool value) const
    {
    }

    void setInt(const char* name, int value) const
    {
    }

    void setVec2(const char* name, float v1, float v2) const noexcept
    {
    }

    void setVec4(const char* name, float v1, float v2, float v3, float v4) const noexcept
    {
    }

    void setFloat(const char* name, float value) const
    {
    }

private:
    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(unsigned int shader, const char* type)
    {
        int  success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {

        }
        else
        {

        }
    }
};