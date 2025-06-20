
#include <cmath>
#include <algorithm>
#include "GaussianBlur.h"

std::vector<float> GaussianBlur::convertPixelArrayToFloatRGB(const std::vector<Pixel>& pixels, int width, int height) {
    std::vector<float> floatData(width * height * 3);

    for (int i = 0; i < width * height; ++i) {
        floatData[3 * i + 0] = static_cast<float>(pixels[i].r);
        floatData[3 * i + 1] = static_cast<float>(pixels[i].g);
        floatData[3 * i + 2] = static_cast<float>(pixels[i].b);
    }

    return floatData;
}

void GaussianBlur::convertFloatRGBToPixelArray(const std::vector<float>& floatData, std::vector<Pixel>& pixels, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        pixels[i].r = static_cast<unsigned char>(std::clamp(floatData[3 * i + 0], 0.0f, 255.0f));
        pixels[i].g = static_cast<unsigned char>(std::clamp(floatData[3 * i + 1], 0.0f, 255.0f));
        pixels[i].b = static_cast<unsigned char>(std::clamp(floatData[3 * i + 2], 0.0f, 255.0f));
    }
}

void GaussianBlur::applyGaussianBlurToImage(Image& image, float sigma) {
    // Convert Pixel array to float RGB
    std::vector<float> floatData = GaussianBlur::convertPixelArrayToFloatRGB(image.pixel_data, image.width, image.height);

    // Apply Gaussian Blur
    GaussianBlur::ApplyToRGB(floatData.data(), image.width, image.height, sigma);

    // Convert back to Pixel array
    GaussianBlur::convertFloatRGBToPixelArray(floatData, image.pixel_data, image.width, image.height);
}

std::vector<float> GaussianBlur::GenerateKernel(float sigma) {
    int radius = static_cast<int>(std::ceil(3.0f * sigma));
    int size = 2 * radius + 1;
    std::vector<float> kernel(size);
    float sum = 0.0f;

    for (int i = -radius; i <= radius; ++i) {
        float value = std::exp(-(i * i) / (2.0f * sigma * sigma));
        kernel[i + radius] = value;
        sum += value;
    }

    // Normalize
    for (float& val : kernel) {
        val /= sum;
    }

    return kernel;
}

void GaussianBlur::ConvolveHorizontal(std::vector<std::vector<float>>& image, const std::vector<float>& kernel) {
    int width = static_cast<int>(image[0].size());
    int height = static_cast<int>(image.size());
    int radius = static_cast<int>(kernel.size()) / 2;

    std::vector<std::vector<float>> temp(height, std::vector<float>(width, 0.0f));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int ix = std::clamp(x + k, 0, width - 1);
                sum += image[y][ix] * kernel[k + radius];
            }
            temp[y][x] = sum;
        }
    }

    image = temp;
}

void GaussianBlur::ConvolveVertical(std::vector<std::vector<float>>& image, const std::vector<float>& kernel) {
    int width = static_cast<int>(image[0].size());
    int height = static_cast<int>(image.size());
    int radius = static_cast<int>(kernel.size()) / 2;

    std::vector<std::vector<float>> temp(height, std::vector<float>(width, 0.0f));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int iy = std::clamp(y + k, 0, height - 1);
                sum += image[iy][x] * kernel[k + radius];
            }
            temp[y][x] = sum;
        }
    }

    image = temp;
}

void GaussianBlur::Apply(std::vector<std::vector<float>>& image, float sigma) {
    std::vector<float> kernel = GenerateKernel(sigma);
    ConvolveHorizontal(image, kernel);
    ConvolveVertical(image, kernel);
}

void GaussianBlur::ApplyToRGB(float* data, int width, int height, float sigma) {
    for (int channel = 0; channel < 3; ++channel) {
        std::vector<std::vector<float>> channelData(height, std::vector<float>(width, 0.0f));

        // Extract channel
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                channelData[y][x] = data[3 * (y * width + x) + channel];
            }
        }

        // Apply Gaussian blur
        Apply(channelData, sigma);

        // Write back
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                data[3 * (y * width + x) + channel] = channelData[y][x];
            }
        }
    }
}
