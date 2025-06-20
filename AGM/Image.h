#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>
#include <iostream>
#include "Pixel.h" // Include Pixel definition

class Image {
public:
    int width;           // Image width
    int height;          // Image height
    std::vector<Pixel> pixel_data; // Pixel pixel_data, stored linearly (row-major order)

    // Constructor to create an empty image of specified dimensions
    Image(int w, int h) : width(w), height(h), pixel_data(w * h) {}

    // Constructor to create an image and optionally load from a file (simplified)
    // Note: For actual image loading/saving (e.g., JPG, PNG), you'd use a library.
    // This example implies raw pixel pixel_data or a simple format.
    Image(const std::string& filename, int w, int h) : width(w), height(h), pixel_data(w * h) {
        // Simplified: In a real scenario, you'd load pixel pixel_data from 'filename' here.
        // For demonstration, we'll assume the pixel_data is either manually set or
        // loaded by a more complex function that uses an image library.
        // For this example, we might fill it with dummy pixel_data or expect main to fill it.
    }

    // finds pixel by row and column
    Pixel findPixel(int row, int col) const {
        if (row >= 0 && row < height && col >= 0 && col < width) {
            return pixel_data[row * width + col];
        }
        // Return a default/black pixel if out of bounds (or throw an error)
        return {0, 0, 0};
    }

    // Get index from row and column
    int index(int row, int col) const {
        return row * width + col;
    }

};

#endif // IMAGE_H