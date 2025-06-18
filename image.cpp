#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "image.h"

Image::Image(const char *filename)
{
    if (read(filename))
    {
        printf("Image loaded successfully: %s\n", filename);
    }
    else
    {
        printf("Failed to load image: %s\n", filename);
    }
}

Image::Image(int w, int h, int channels)
    : w(w), h(h), channels(channels)
{
    size = w * h * channels;
    data = (uint8_t *)STBI_MALLOC(size);
}

Image::Image(const Image &other)
    : Image(w, h, channels)
{
    if (other.data)
    {
        data = (uint8_t *)STBI_MALLOC(size);
        if (data)
        {
            memcpy(data, other.data, size);
        }
    }
    else
    {
        data = NULL;
    }
}

Image::~Image()
{
    if (data)
    {
        stbi_image_free(data);
        data = NULL;
    }
}

bool Image::read(const char *filename)
{
    if (data)
    {
        STBI_FREE(data);
        data = NULL;
    }

    data = stbi_load(filename, &w, &h, &channels, 0);
    if (!data)
    {
        return false;
    }

    size = w * h * channels;
    return true;
}

bool Image::write(const char *filename)
{
    if (!data || size == 0)
    {
        return false;
    }
    int result = 0;
    switch (getFileFormat(filename))
    {
    case FORMAT_PNG:
        result = stbi_write_png(filename, w, h, channels, data, w * channels);
        break;
    case FORMAT_BMP:
        result = stbi_write_bmp(filename, w, h, channels, data);
        break;
    case FORMAT_TGA:
        result = stbi_write_tga(filename, w, h, channels, data);
        break;
    case FORMAT_JPG:
        result = stbi_write_jpg(filename, w, h, channels, data, 100); // Default quality set to 100
        break;
    case FORMAT_HDR:
        result = stbi_write_hdr(filename, w, h, channels, (float *)data);
        break;
    default:
        printf("Unknown or unsuported image format for file: %s\n", filename);
        return false;
    }
    return result != 0;
}

ImageFormat Image::getFileFormat(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if (ext != nullptr)
    {
        if (strcmp(ext, ".png") == 0)
        {
            return FORMAT_PNG;
        }
        else if (strcmp(ext, ".bmp") == 0)
        {
            return FORMAT_BMP;
        }
        else if (strcmp(ext, ".tga") == 0)
        {
            return FORMAT_TGA;
        }
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        {
            return FORMAT_JPG;
        }
        else if (strcmp(ext, ".hdr") == 0)
        {
            return FORMAT_HDR;
        }
    }

    return FORMAT_UNKNOWN;
}