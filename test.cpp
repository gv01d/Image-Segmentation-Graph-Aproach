#include "image.cpp"
#include "edgeCost.cpp"
#include "tinyfiledialogs.h"

#include <map>     // for std::map
#include <utility> // for std::pair
#include <queue>   // for std::queue
#include <set>

std::vector<int> getEdjacentVertices(int pos, int width, int height, bool useDiagonal)
{
    std::vector<int> result;
    int x = pos % width;
    int y = pos / width;

    if (y > 0)
    {
        result.push_back(pos - width);
        if (useDiagonal)
        {
            if (x > 0)
            {
                result.push_back(pos - width - 1);
            }
            if (x < width - 1)
            {
                result.push_back(pos - width + 1);
            }
        }
    }
    if (x > 0)
    {
        result.push_back(pos - 1);
    }
    if (x < width - 1)
    {
        result.push_back(pos + 1);
    }
    if (y < height - 1)
    {
        result.push_back(pos + width);
        if (useDiagonal)
        {
            if (x > 0)
            {
                result.push_back(pos + width - 1);
            }
            if (x < width - 1)
            {
                result.push_back(pos + width + 1);
            }
        }
    }

    return result;
}

class CM
{
public:
    Image image;
    bool useDiagonal = false; // Use diagonal connections if true

    std::map<int, int> seeds; // Map of seed positions to their labels
    std::vector<int> labels;  // Labels for each pixel in the image
    std::vector<float> costs; // costs for each pixel in the image
    std::vector<int> parent;  // Parent pixel for each pixel in the image

    std::queue<int> queue; // Queue for BFS or Dijkstra's algorithm

    EdgeCost *edgeCost = nullptr; // Pointer to the edge cost function

    // _________________________________________________________________________________________________________________
    /// @brief Constructor for the CM class
    CM(Image image, std::map<int, int> seeds, bool useDiagonal = false)
        : image(image), useDiagonal(useDiagonal), labels(image.w * image.h, -1), costs(image.w * image.h, std::numeric_limits<float>::infinity()), parent(image.w * image.h, -1)
    {
        // Initialize labels based on seeds
        for (const auto &seed : seeds)
        {
            if (seed.first < 0 || seed.first >= image.w * image.h)
                continue; // Skip invalid seed positions

            labels[seed.first] = seed.second;
            costs[seed.first] = 0.0f; // Set initial cost for seed pixels
            queue.push(seed.first);

            this->seeds.insert_or_assign(seed.first, seed.second); // add seed to the map
        }
    }

    // _________________________________________________________________________________________________________________
    /// @brief Run the connected components algorithm using BFS or Dijkstra's algorithm
    /// @details This function processes the queue, updating labels and costs for each pixel based on the edge cost.
    void run()
    {
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();

            int currentLabel = labels[current];
            float currentCost = costs[current];

            std::vector<int> neighbors = getEdjacentVertices(current, image.w, image.h, useDiagonal);

            // - - - - - - - - - - - - - - - - - - - - - - - -
            // Process each neighbor of the current pixel
            for (int neighbor : neighbors)
            {
                if (neighbor < 0 || neighbor >= static_cast<int>(labels.size()))
                    continue; // Skip out-of-bounds neighbors

                if (labels[neighbor] == -1) // If the neighbor is unvisited
                {
                    float edgeCostValue = edgeCost ? edgeCost->getCost(current, neighbor) : 1.0f; // Default cost if no edge cost function is provided
                    float newCost = currentCost + edgeCostValue;

                    if (newCost < costs[neighbor]) // If the new cost is lower than the previous cost
                    {
                        costs[neighbor] = newCost;
                        labels[neighbor] = currentLabel; // Assign the label of the current pixel to the neighbor
                        parent[neighbor] = current;      // Set the parent of the neighbor to the current pixel
                        queue.push(neighbor);            // Add the neighbor to the queue for further processing
                    }
                }
            }
        }
    }
};

int xyToPos(Image image, int x, int y)
{
    if (x < 0 || x >= image.w || y < 0 || y >= image.h)
        return -1;          // Invalid position
    return y * image.w + x; // Convert (x, y) to a linear index
}

const char *filters[] = {"*.png", "*.jpg", "*.bmp", "*.tga", "*.hdr"};

int main()
{
    // Example usage
    const char *text = "images\\random.jpg";
    if (true)
    {
        text = tinyfd_openFileDialog(
            "Select a gml file",                       // Dialog title
            "",                                        // Default path and/or filename
            1,                                         // Number of filter patterns
            filters,                                   // Filter patterns
            "image files - (png, jpg, bmp, tga, hdr)", // Optional filter description (can be NULL)
            0                                          // Allow multiple select (0 = no)
        );
    }

    Image image(text);                // Load an image from a file
    image.write("output\\input.png"); // Write the image to a file

    // Generate 10 arbitrary seed spots based on image width and height
    std::map<int, int> seeds;
    int w = image.w, h = image.h;
    std::vector<std::pair<float, float>> relCoords = {
        {0.1f, 0.1f}, //
        {0.2f, 0.8f},
        {0.4f, 0.3f},
        {0.6f, 0.7f},
        {0.8f, 0.2f},
        {0.9f, 0.9f},
        {0.3f, 0.6f},
        {0.7f, 0.4f},
        {0.5f, 0.5f},
        {0.85f, 0.75f} //
    };
    for (int i = 0; i < 10; ++i)
    {
        int x = static_cast<int>(relCoords[i].first * (w - 1));
        int y = static_cast<int>(relCoords[i].second * (h - 1));
        int pos = xyToPos(image, x, y);
        if (pos != -1)
            seeds[pos] = i + 1;
    }

    // Generate random colors for each seed label, ensuring they are different
    std::map<int, std::tuple<uint8_t, uint8_t, uint8_t>> labelColors;
    std::set<std::tuple<uint8_t, uint8_t, uint8_t>> usedColors;
    for (const auto &seed : seeds)
    {
        int label = seed.second;
        std::tuple<uint8_t, uint8_t, uint8_t> color;
        do
        {
            uint8_t r = static_cast<uint8_t>(rand() % 256);
            uint8_t g = static_cast<uint8_t>(rand() % 256);
            uint8_t b = static_cast<uint8_t>(rand() % 256);
            color = std::make_tuple(r, g, b);
        } while (usedColors.count(color) > 0);
        labelColors[label] = color;
        usedColors.insert(color);
    }

    EuclidianDistance_EdgeCost edgeCost(image); // Create an edge cost object

    CM cm(image, seeds, true); // Create a CM object with diagonal connections
    cm.edgeCost = &edgeCost;   // Set the edge cost function

    // _________________________________________________________________________________________________
    // Run the connected components algorithm

    cm.run(); // Run the connected components algorithm

    // _________________________________________________________________________________________________
    // Output

    Image outputImage(image.w, image.h, 3); // Create an output image with 3 channels (RGB)
    int lastLabel = -1;                     // Initialize last label to -1
    for (int i = 0; i < image.w * image.h; ++i)
    {
        int label = cm.labels[i];

        if (label != -1) // If the pixel has a label
        {
            auto color = labelColors[label]; // Get the color for the label
            /*
            if (lastLabel != label)          // If the color is different from the last color
            {
                lastLabel = label; // Update the last color if it is different
                printf("Label %d: Color (%d, %d, %d)\n", label, std::get<0>(color), std::get<1>(color), std::get<2>(color));
            }
            */

            outputImage.data[i * 3] = std::get<0>(color);     // Set red channel
            outputImage.data[i * 3 + 1] = std::get<1>(color); // Set green channel
            outputImage.data[i * 3 + 2] = std::get<2>(color); // Set blue channel
        }
    }

    outputImage.write("output\\output.png"); // Write the output image to a file
    printf("Output image written to output\\output.png\n");

    return 0;
}