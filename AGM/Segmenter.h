#ifndef SEGMENTER_H
#define SEGMENTER_H

#include "Image.h"
#include "Pixel.h"
#include "Edge.h"
#include "Disjoint.h"

#include <vector>
#include <queue>
#include <unordered_map>

class Segmenter {
public:
    const Image& image; // Reference to the input image
    int width;           // Image width
    int height;          // Image height

    // Constructor: Initializes the segmenter with the input image.
    Segmenter(const Image& img);

    // Calculates the color difference between two pixels (used by Felzenszwalb).
    double rgbDistance(const Pixel& a, const Pixel& b);

    // Builds a list of all edges (pixel pairs) with their calculated weights.
    // Uses 4-connectivity (horizontal and vertical neighbors).
    std::vector<Edge> createGraph();

    // Performs image segmentation using the Felzenszwalb algorithm.
    // 'k' is the scale parameter.
    std::vector<int> segment(double k);

    // Visualizes the segmentation by assigning random colors to each segment.
    // Returns a new Image object with the colored segments.
    Image segmentationVisualization(const std::vector<int>& labels);

};

#endif // SEGMENTER_H