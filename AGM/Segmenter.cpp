#include "Segmenter.h"
#include "Disjoint.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <set>
#include <limits>

// Constructor for the Segmenter class. Initializes with the provided image.
Segmenter::Segmenter(const Image& img) : image(img), width(img.width), height(img.height) {}

//
Image Segmenter::segmentationVisualization(const std::vector<int>& labels) {
    Image output_image(width, height); // Create blank image with same dimensions
    
    // 1. Find all unique labels first
    std::set<int> unique_labels(labels.begin(), labels.end());
    
    // 2. Create a deterministic color mapping based on label values
    std::unordered_map<int, Pixel> label_colors;
    int counter = 0;
    
    for (int label : unique_labels) {
        // Create perceptually distinct colors
        unsigned char r = (counter * 67) % 256;
        unsigned char g = (counter * 179) % 256;
        unsigned char b = (counter * 241) % 256;
        label_colors[label] = {r, g, b};
        counter++;
    }

    // 3. Apply colors to output image
    for (size_t i = 0; i < labels.size(); ++i) {
        output_image.pixel_data[i] = label_colors[labels[i]];
    }

    return output_image;
}

// Calculates the Euclidean distance between two pixels in RGB color space.
// Used for edge weights in the Felzenszwalb algorithm.
double Segmenter::rgbDistance(const Pixel& pix_a, const Pixel& pix_b) {
    double red_dist = pix_a.r - pix_b.r;
    red_dist*=red_dist;

    double grn_dist = pix_a.g - pix_b.g;
    grn_dist*=grn_dist;

    double blue_dist = pix_a.b - pix_b.b;
    blue_dist*=blue_dist;

    return sqrt(red_dist + grn_dist + blue_dist);
}

// Implements the Felzenszwalb graph-based segmentation algorithm.
// 'k' controls the scale of segmentation.
std::vector<int> Segmenter::segment(double k) {
    int total_pixels = width * height;
    
    std::vector<Edge> graph = createGraph(); // Get all pixel edges
    // Sort edges by weight in ascending order
    std::sort(graph.begin(), graph.end(), [](Edge e1, Edge e2) 
    { return e1.weight < e2.weight; });
    Disjoint disjoint_sets(total_pixels); // Initialize Disjoint Set Union

    // Iterate through sorted edges and apply the merging criterion
    for (const Edge& current_edge : graph) {
        // Attempt to unite the sets of the two pixels connected by the edge.
        // The DSU's unite_sets function internally checks the Felzenszwalb merging condition.

        int root1 = disjoint_sets.find_set_root(current_edge.u); 
        int root2 = disjoint_sets.find_set_root(current_edge.v); 

        if (root1 != root2) { // if roots are different
            // Calculate the adaptive thresholds
            double tau1 = k / disjoint_sets.component_size[root1];
            double tau2 = k / disjoint_sets.component_size[root2];

            // Calculate the minimum internal difference (MInt)
            double mInt = std::min(disjoint_sets.max_internal_edge[root1] + tau1, disjoint_sets.max_internal_edge[root2] + tau2);

            if (current_edge.weight <= mInt) { // If the edge weight is less than or equal to MInt, merge
                disjoint_sets.unite_sets(root1, root2, current_edge.weight);
            }
        }

    }

    std::vector<int> regions(total_pixels);
    // Assign labels to regions using the Disjoint Set Union
    for (int i = 0; i < total_pixels; ++i) {
        regions[i] = disjoint_sets.find_set_root(i);
    }

    std::unordered_map<int, int> region_sizes;
    for (int region : regions) region_sizes[region]++;

    return regions;
}

// Builds a graph, vector of all edges between 4-connected neighboring pixels.
std::vector<Edge> Segmenter::createGraph() {
    std::vector<Edge> edges_list;
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int current_pixel_idx = image.index(r, c);
            
            // Connect to right neighbor
            if (c + 1 < width) {
                int right_neighbor_idx = image.index(r, c + 1);
                edges_list.push_back({current_pixel_idx, right_neighbor_idx, rgbDistance(image.findPixel(r, c), image.findPixel(r, c + 1))});
            }
            // Connect to bottom neighbor
            if (r + 1 < height) {
                int bottom_neighbor_idx = image.index(r + 1, c);
                edges_list.push_back({current_pixel_idx, bottom_neighbor_idx, rgbDistance(image.findPixel(r, c), image.findPixel(r + 1, c))});
            }
        }
    }
    return edges_list;
}
