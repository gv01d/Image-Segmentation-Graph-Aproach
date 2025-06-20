#include "image/image.cpp"

class gradient
{
public:
    static int sumChannels(uint8_t *pixel, int channels)
    {
        int ret = 0;
        for (size_t i = 0; i < channels; i++)
        {
            ret += pixel[i];
        }
        return ret / channels;
    }

    static Image generateGradient(Image image)
    {
        // Sobel kernels
        int gx[3][3] = {
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}};
        int gy[3][3] = {
            {-1, -2, -1},
            {0, 0, 0},
            {1, 2, 1}};

        int width = image.w;
        int height = image.h;
        Image result(width, height, 1);

        for (int y = 1; y < height - 1; ++y)
        {
            for (int x = 1; x < width - 1; ++x)
            {
                int sumX = 0;
                int sumY = 0;
                for (int ky = -1; ky <= 1; ++ky)
                {
                    for (int kx = -1; kx <= 1; ++kx)
                    {
                        uint8_t *pixel = image.getPixel(x + kx, y + ky);
                        int pVal = sumChannels(pixel, image.channels);
                        sumX += gx[ky + 1][kx + 1] * pVal;
                        sumY += gy[ky + 1][kx + 1] * pVal;
                    }
                }
                int magnitude = static_cast<int>(std::sqrt(sumX * sumX + sumY * sumY));
                magnitude = std::min(255, std::max(0, magnitude));
                uint8_t mag = (uint8_t)magnitude;
                result.setPixel(x, y, &mag);
            }
        }
        return result;
    }
};