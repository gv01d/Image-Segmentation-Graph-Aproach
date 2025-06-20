#include "src/dijkstra.cpp"
#include "src/tinyfiledialogs.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include "src/gradient.cpp"

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

// Converte a posição linear de volta para coordenadas (x, y)
std::pair<int, int> posToXy(int pos, int width)
{

    if (width == 0)
        return {-1, -1};
    return {pos % width, pos / width};
}

//---| CRUD DE SEEDS |---

//--| READ |--

void exibirSeeds(const std::map<int, int> &seeds, int imageWidth)
{
    std::cout << "\n---| Seeds Atuais |---" << std::endl;
    if (seeds.empty())
    {
        std::cout << "Nenhuma seed definida." << std::endl;
    }
    else
    {

        for (std::map<int, int>::const_iterator it = seeds.begin(); it != seeds.end(); ++it)
        {
            int pos = it->first;
            int label = it->second;

            std::pair<int, int> coords = posToXy(pos, imageWidth);
            std::cout << "Seed " << label << ": Coordenadas (" << coords.first << ", " << coords.second << ")" << std::endl;
        }
    }
    std::cout << "--------------------\n"
              << std::endl;
}

// --| CRUD |--
void crudSeeds(std::map<int, int> &seeds, const Image &image)
{
    int choice = 0;

    while (true)
    {

        std::cout << "\n----| CRUD de Seeds |----" << std::endl;
        std::cout << "1. Adicionar nova seed" << std::endl;
        std::cout << "2. Exibir seeds atuais" << std::endl;
        std::cout << "3. Atualizar uma seed" << std::endl;
        std::cout << "4. Remover uma seed" << std::endl;
        std::cout << "5. Concluir e executar o algoritmo" << std::endl;
        std::cout << "Escolha uma opcao: ";

        // Validação de entrada numérica
        while (!(std::cin >> choice))
        {
            std::cout << "Entrada invalida.";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if (choice == 5)
        {
            if (seeds.empty())
            {
                std::cout << "AVISO: Nenhuma seed foi definida." << std::endl;
            }
            std::cout << "Finalizando definicao de seeds" << std::endl;

            return;
        }

        switch (choice)
        {
        // --| CREATE |--
        case 1:
        {
            int x, y;
            std::cout << "Digite a coordenada X (0-" << image.w - 1 << "): ";
            std::cin >> x;
            std::cout << "Digite a coordenada Y (0-" << image.h - 1 << "): ";
            std::cin >> y;

            int pos = xyToPos(image, x, y);
            if (pos == -1)
            {
                std::cerr << "Erro: Coordenadas fora dos limites da imagem." << std::endl;
            }
            else
            {

                int newLabel = 1;
                if (!seeds.empty())
                {

                    // Encontra o maior rótulo atual e adiciona 1
                    auto max_it = std::max_element(seeds.begin(), seeds.end(),
                                                   [](const auto &a, const auto &b)
                                                   { return a.second < b.second; });
                    newLabel = max_it->second + 1;
                }
                seeds[pos] = newLabel;
                std::cout << "Seed " << newLabel << " adicionada em (" << x << ", " << y << ")." << std::endl;
            }
            break;
        }

        //--| READ |--
        case 2:
        {
            exibirSeeds(seeds, image.w);
            break;
        }
        //--| UPDATE |--
        case 3:
        {
            if (seeds.empty())
            {
                std::cout << "Nenhuma seed para atualizar." << std::endl;
                break;
            }

            // atualizazndo seed
            exibirSeeds(seeds, image.w);
            int label;
            std::cout << "Digite o numero (label) da seed que deseja atualizar: ";
            std::cin >> label;

            auto it = std::find_if(seeds.begin(), seeds.end(),
                                   [label](const auto &pair)
                                   { return pair.second == label; });

            if (it == seeds.end())
            {
                std::cerr << "Erro: Seed com label " << label << " nao encontrada." << std::endl;
            }
            else
            {

                int oldPos = it->first;
                int x, y;
                std::cout << "Digite a NOVA coordenada X: ";
                std::cin >> x;
                std::cout << "Digite a NOVA coordenada Y: ";
                std::cin >> y;

                int newPos = xyToPos(image, x, y);
                if (newPos == -1)
                {
                    std::cerr << "Erro: Coordenadas fora dos limites." << std::endl;
                }
                else
                {
                    seeds.erase(it);
                    seeds[newPos] = label;
                    std::cout << "Seed " << label << " atualizada para (" << x << ", " << y << ")." << std::endl;
                }
            }
            break;
        }

        //--| DELETE |--
        case 4:
        {
            if (seeds.empty())
            {
                std::cout << "Nenhuma seed para remover." << std::endl;
                break;
            }
            exibirSeeds(seeds, image.w);
            int label;
            std::cout << "Digite o numero (label) da seed que deseja remover: ";
            std::cin >> label;

            auto it = std::find_if(seeds.begin(), seeds.end(),
                                   [label](const auto &pair)
                                   { return pair.second == label; });

            if (it == seeds.end())
            {
                std::cerr << "Erro: Seed com label " << label << " nao encontrada." << std::endl;
            }
            else
            {
                seeds.erase(it);
                std::cout << "Seed " << label << " removida." << std::endl;
            }
            break;
        }
        default:
            std::cout << "Opcao invalida. Tente novamente." << std::endl;
            break;
        }
    }
}

const char *filters[] = {"*.png", "*.jpg", "*.bmp", "*.tga", "*.hdr"};

int main()
{
    // Example usage
    const char *text = "images\\random.jpg";
    if (true)
    {
        text = tinyfd_openFileDialog(
            "Select a image file",     // Dialog title
            "",                        // Default path and/or filename
            1,                         // Number of filter patterns
            filters,                   // Filter patterns
            "png, jpg, bmp, tga, hdr", // Optional filter description (can be NULL)
            0                          // Allow multiple select (0 = no)
        );
    }

    if (!text)
    {
        std::cerr << "Nenhum arquivo selecionado. Encerrando." << std::endl;
        return 1;
    }

    Image image(text);                // Load an image from a file
    image.write("output\\input.png"); // Write the image to a file

    Image imageGradient = gradient::generateGradient(image);
    imageGradient.write("output\\gradient.png");

    // Generate 10 arbitrary seed spots based on image width and height
    std::map<int, int> seeds;

    // Força uma pausa e limpa completamente o buffer de entrada
    std::string dummy;
    std::cout << "\n>>> Imagem carregada. Pressione ENTER para abrir o menu de seeds <<<" << std::endl;
    std::getline(std::cin, dummy); // Limpa qualquer sobra anterior
    std::getline(std::cin, dummy); // Aguarda o ENTER real

    // Chamando funcao de CRUD
    std::cout << "Abrindo o CRUD " << std::endl;
    crudSeeds(seeds, imageGradient);

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

    EuclidianDistance_EdgeCost edgeCost(imageGradient); // Create an edge cost object

    CM cm(imageGradient, seeds, true); // Create a CM object with diagonal connections
    cm.edgeCost = &edgeCost;           // Set the edge cost function

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