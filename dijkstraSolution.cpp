#include "src/dijkstra.cpp"
#include "src/tinyfiledialogs.h"

// For Windows-specific functionality (ShellExecute) - Optional, but more robust
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

void openPhoto(const std::string &imagePath)
{
#ifdef _WIN32
    // Windows: Use ShellExecuteW for Unicode support
    // Convert std::string to std::wstring
    auto s2ws = [](const std::string &str) -> std::wstring
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    };
    std::wstring wImagePath = s2ws(imagePath);
    HINSTANCE result = ShellExecuteW(NULL, L"open", wImagePath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    if ((intptr_t)result <= 32)
    { // ShellExecute returns a value <= 32 on error
        std::cerr << "Error: Could not open image on Windows. Error code: " << (intptr_t)result << std::endl;
        std::cerr << "Make sure the file path is correct and a default image viewer is set." << std::endl;
    }
    else
    {
        std::cout << "Successfully launched image on Windows." << std::endl;
    }

#elif __APPLE__
    // macOS: Use the 'open' command
    std::string command = "open \"" + imagePath + "\"";
    std::cout << "Executing command: " << command << std::endl;
    int result = system(command.c_str());
    if (result != 0)
    {
        std::cerr << "Error: Could not open image on macOS. System command returned: " << result << std::endl;
        std::cerr << "Make sure the file path is correct." << std::endl;
    }
    else
    {
        std::cout << "Successfully launched image on macOS." << std::endl;
    }

#elif __linux__
    // Linux: Try common commands. 'xdg-open' is the most generic and recommended.
    // Fallback to 'gnome-open', 'kde-open', or just the file path if xdg-open fails.
    std::string command = "xdg-open \"" + imagePath + "\"";
    std::cout << "Executing command: " << command << std::endl;
    int result = system(command.c_str());
    if (result != 0)
    {
        std::cerr << "Warning: xdg-open failed. Trying 'gnome-open'..." << std::endl;
        command = "gnome-open \"" + imagePath + "\"";
        result = system(command.c_str());
        if (result != 0)
        {
            std::cerr << "Warning: gnome-open failed. Trying 'kde-open'..." << std::endl;
            command = "kde-open \"" + imagePath + "\"";
            result = system(command.c_str());
            if (result != 0)
            {
                std::cerr << "Error: Could not open image on Linux using common methods. System command returned: " << result << std::endl;
                std::cerr << "Make sure the file path is correct and a default image viewer is set (e.g., using 'xdg-mime' commands)." << std::endl;
            }
            else
            {
                std::cout << "Successfully launched image using kde-open." << std::endl;
            }
        }
        else
        {
            std::cout << "Successfully launched image using gnome-open." << std::endl;
        }
    }
    else
    {
        std::cout << "Successfully launched image on Linux using xdg-open." << std::endl;
    }

#else
    std::cerr << "Unsupported operating system. Cannot automatically open photo." << std::endl;
#endif
}

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