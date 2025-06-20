#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "Segmenter.h"
#include "GaussianBlur.h"


// --- STB_IMAGE INTEGRATION ---
// Define these before including stb_image headers in ONE .cpp file (e.g., main.cpp)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // For loading images

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // For saving images (e.g., PNG)
// --- END STB_IMAGE INTEGRATION ---


// Modified function to load an image using stb_image
Image loadImageFromFile(const std::string& filename) {
    int width, height, channels;
    unsigned char* img_data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb); // Force 3 channels (RGB)

    if (!img_data) {
        std::cerr << "Error: Could not load image from " << filename << std::endl;
        return Image(0, 0); // Return an empty image
    }

    Image loaded_image(width, height);
    for (int i = 0; i < width * height; ++i) {
        loaded_image.pixel_data[i].r = img_data[i * 3 + 0];
        loaded_image.pixel_data[i].g = img_data[i * 3 + 1];
        loaded_image.pixel_data[i].b = img_data[i * 3 + 2];
    }

    stbi_image_free(img_data); // Free the loaded image data
    std::cout << "Successfully loaded image: " << filename << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
    return loaded_image;
}

// Function to save an image using stb_image (e.g., as PNG)
bool saveImageToFile(const Image& img, const std::string& filename) {
    std::vector<unsigned char> output_data(img.width * img.height * 3);
    for (int i = 0; i < img.width * img.height; ++i) {
        output_data[i * 3 + 0] = img.pixel_data[i].r;
        output_data[i * 3 + 1] = img.pixel_data[i].g;
        output_data[i * 3 + 2] = img.pixel_data[i].b;
    }

    // Save as PNG
    if (stbi_write_png(filename.c_str(), img.width, img.height, 3, output_data.data(), img.width * 3)) {
        std::cout << "Image saved to " << filename << std::endl;
        return true;
    } else {
        std::cerr << "Error: Could not save image to " << filename << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    // 1. Load the input image
    std::string input_image_path = "n sei.png";
    Image input_image = loadImageFromFile(input_image_path);
    if (input_image.width == 0 || input_image.height == 0) {
        return 1; // Exit if image couldn't be loaded
    }

    input_image_path = "n sei.png";
    Image input_image_g = loadImageFromFile(input_image_path);//loads greyscale image
    if (input_image.width == 0 || input_image.height == 0) {
        return 1; // Exit if image couldn't be loaded
    }

    // 2. Applies gaussian blur
    GaussianBlur::applyGaussianBlurToImage(input_image, 0.8f);
    GaussianBlur::applyGaussianBlurToImage(input_image_g, 0.8f);

    // 3. Runs Felzenszwalb segmentation algorithm
    double k = 500.0; // controls segment size, higher->less segments
    Segmenter segmenter(input_image);
    std::vector<int> labels = segmenter.segment(k);
    Segmenter segmenter_g(input_image_g);
    std::vector<int> labels_g = segmenter_g.segment(k);

    // 4. Saves image
    Image segmentation_output = segmenter.segmentationVisualization(labels);
    saveImageToFile(segmentation_output, "segmentation_output.png");
    Image segmentation_output_g = segmenter_g.segmentationVisualization(labels_g);
    saveImageToFile(segmentation_output_g, "segmentation_output_g.png");

    return 0;
}