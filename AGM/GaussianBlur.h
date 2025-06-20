#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H
#include <vector>
#include "Image.h"

class GaussianBlur {
public:
    // Applies Gaussian blur to a grayscale image
    static void Apply(std::vector<std::vector<float>>& image, float sigma);

    // Applies Gaussian blur to an RGB image
    // The data is assumed to be a flat array in RGBRGB... order
    static void ApplyToRGB(float* data, int width, int height, float sigma);

    static std::vector<float> convertPixelArrayToFloatRGB(const std::vector<Pixel>& pixels, int width, int height);

    static void convertFloatRGBToPixelArray(const std::vector<float>& floatData, std::vector<Pixel>& pixels, int width, int height);

    static void applyGaussianBlurToImage(Image& image, float sigma);

    // Generates a 1D Gaussian kernel for given sigma
    static std::vector<float> GenerateKernel(float sigma);

    // Applies 1D convolution horizontally
    static void ConvolveHorizontal(std::vector<std::vector<float>>& image, const std::vector<float>& kernel);

    // Applies 1D convolution vertically
    static void ConvolveVertical(std::vector<std::vector<float>>& image, const std::vector<float>& kernel);
};

#endif // GAUSSIAN_BLUR_H
