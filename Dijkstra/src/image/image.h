#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <cstdio>

enum ImageFormat
{
    FORMAT_UNKNOWN = 0,
    FORMAT_PNG,
    FORMAT_BMP,
    FORMAT_TGA,
    FORMAT_JPG,
    FORMAT_HDR
};

struct Image
{
    uint8_t *data = NULL; // Pointer to the image data
    size_t size = 0;      // Size of the image data in bytes
    int w;                // Width of the image in pixels
    int h;                // Height of the image in pixels
    int channels;         // Number of color channels in the image (e.g., 3 for RGB, 4 for RGBA)

    Image(const char *filename);
    Image(int w, int h, int channels);
    Image(const Image &other);
    ~Image();

    bool read(const char *filename);
    bool write(const char *filename);

    uint8_t *getPixel(int x, int y);
    void setPixel(int x, int y, uint8_t *data);

    ImageFormat getFileFormat(const char *filename);
};
#endif // IMAGE_H