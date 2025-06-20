#include "edgeCost.cpp"

#include <map>     // for std::map
#include <utility> // for std::pair
#include <queue>   // for std::queue
#include <set>
#include <string>
#include <iostream>

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

struct PixelNode
{
    int index;  // Linear index of pixel
    float cost; // Cost to reach this pixel

    // Reverse order for min-heap behavior
    bool operator<(const PixelNode &other) const
    {
        return cost > other.cost;
    }
};

class CM
{
public:
    Image image;
    bool useDiagonal = false; // Use diagonal connections if true

    std::map<int, int> seeds; // Map of seed positions to their labels
    std::vector<int> labels;  // Labels for each pixel in the image
    std::vector<float> costs; // costs for each pixel in the image
    std::vector<int> parent;  // Parent pixel for each pixel in the image

    std::priority_queue<PixelNode> queue; // Queue for BFS or Dijkstra's algorithm

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
            queue.push(PixelNode{seed.first, 0.0f});

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
            int current = queue.top().index; // Get the pixel with the lowest cost from the queue
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
                        labels[neighbor] = currentLabel;          // Assign the label of the current pixel to the neighbor
                        parent[neighbor] = current;               // Set the parent of the neighbor to the current pixel
                        queue.push(PixelNode{neighbor, newCost}); // Add the neighbor to the queue for further processing
                    }
                }
            }
        }
    }
};