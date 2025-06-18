
#include "image.h"

#include <limits> // Required for std::numeric_limits
#include <cmath>  // Required for sqrtf
#include <vector>

class EdgeCost
{
protected:
    Image image;

public:
    EdgeCost(const Image &image) : image(image) {}
    virtual float getCost(int from, int to) const = 0;
    virtual ~EdgeCost() = default;
};

class EuclidianDistance_EdgeCost : public EdgeCost
{
public:
    EuclidianDistance_EdgeCost(const Image &image) : EdgeCost(image) {}

    float getCost(int from, int to) const override
    {
        // Assuming the image data is in RGB format
        uint8_t *data = image.data;
        int channels = image.channels;

        if (from < 0 || from >= image.w * image.h || to < 0 || to >= image.w * image.h)
        {
            return std::numeric_limits<float>::infinity(); // Return a large cost for invalid indices
        }

        std::vector<int> fromCols(channels);
        std::vector<int> toCols(channels);

        for (int i = 0; i < channels; ++i)
        {
            fromCols[i] = data[from * channels + i];
            toCols[i] = data[to * channels + i];
        }

        float distanceSquared = 0;
        for (int i = 0; i < channels; ++i)
        {
            float diff = fromCols[i] - toCols[i];
            distanceSquared += diff * diff;
        }

        // Calculate Euclidean distance in RGB space
        return sqrtf(distanceSquared);
    }
};