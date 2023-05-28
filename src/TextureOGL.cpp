#include "Engine/TextureOGL.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char* srcPath, std::function<void()> setupCallback)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    setupCallback();

    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load(srcPath, &width, &height, &nbChannels, 0);
    if (data)
    {
        if (nbChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        log("Failed to load texture");
    }
}

Texture::Texture(void* data, int pxlWidth, int pxlHeight, int channels, std::function<void()> setupCallback)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    setupCallback();

    width           = pxlWidth;
    height          = pxlHeight;
    nbChannels      = channels;
    GLenum chanEnum = getChanelEnum();
    glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
}

Texture::Texture(int pxlWidth, int pxlHeight, int channels, std::function<void()> setupCallback)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    setupCallback();

    width           = pxlWidth;
    height          = pxlHeight;
    nbChannels      = channels;
    GLenum chanEnum = getChanelEnum();

    glTexImage2D(GL_TEXTURE_2D, 0, chanEnum, width, height, 0, chanEnum, GL_UNSIGNED_BYTE, 0);
}

Texture::~Texture()
{
    if (data != nullptr)
        stbi_image_free(data);
    glDeleteTextures(1, &ID);
}