// Render Image Centered with OpenGL + GLAD + GLFW
// Loads image using stb_image.h

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../image/image.cpp"
#include <iostream>
#include <vector>
#include "../tinyfiledialogs.h"
#include "../dijkstra.cpp"
#include "../gradient.cpp"

// Vertex and fragment shaders
const char *vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
out vec2 TexCoords;
void main() {
    TexCoords = aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char *fragmentShaderSrc = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D image;
void main() {
    FragColor = texture(image, TexCoords);
})";

const char *pointVertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char *pointFragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Red with fadeout
}
)";

Image *mainImage;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLuint createShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

GLFWwindow *initGLFW_GLAD(int width, int height, const char *title)
{
    if (!glfwInit())
        return nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return window;
}

GLuint loadTexture(const char *filename)
{
    stbi_set_flip_vertically_on_load(1);
    mainImage = new Image(filename);
    if (!mainImage->data)
        return 0;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, mainImage->channels == 4 ? GL_RGBA : GL_RGB,
                 mainImage->w, mainImage->h, 0,
                 mainImage->channels == 4 ? GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE, mainImage->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(mainImage->data);
    return texture;
}

void createQuadVAO(float scaleX, float scaleY, GLuint &VAO, GLuint &VBO, GLuint &EBO)
{
    float vertices[] = {
        -scaleX, scaleY, 0.0f, 1.0f,
        -scaleX, -scaleY, 0.0f, 0.0f,
        scaleX, -scaleY, 1.0f, 0.0f,
        scaleX, scaleY, 1.0f, 1.0f};
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void createPoints(float *points, int amount, GLuint &VAO, GLuint &VBO)
{
    // points: array of [x, y] pairs in normalized device coordinates (-1 to 1)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, amount * 2 * sizeof(float), points, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

GLuint createShaderProgram(const char *vertexSrc, const char *fragmentSrc)
{
    GLuint vs = createShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = createShader(GL_FRAGMENT_SHADER, fragmentSrc);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void updateQuadVBO(float scaleX, float scaleY, GLuint VBO)
{
    float vertices[] = {
        -scaleX, scaleY, 0.0f, 1.0f,
        -scaleX, -scaleY, 0.0f, 0.0f,
        scaleX, -scaleY, 1.0f, 0.0f,
        scaleX, scaleY, 1.0f, 1.0f};
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void mainLoop(GLFWwindow *window, GLuint program, GLuint pointShader, GLuint texture, GLuint VAO, GLuint VBO, float border, const char *filename)
{
    glUniform1i(glGetUniformLocation(program, "image"), 0);
    int screenWidth, screenHeight;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        // ________________________________________________________________
        glUseProgram(program);
        // --- Quad calculation and update ---
        float imgAspect = (float)mainImage->w / mainImage->h;
        float drawWidth, drawHeight;
        float emuWidth, emuHeight;
        if (screenWidth < screenHeight)
        {
            drawWidth = screenWidth;
            drawHeight = drawWidth / imgAspect;

            emuWidth = screenWidth;
            emuHeight = emuWidth / imgAspect;
        }
        else
        {
            drawHeight = screenHeight;
            drawWidth = drawHeight * imgAspect;

            emuHeight = screenHeight;
            emuWidth = emuHeight * imgAspect;
        }

        float Rw = emuWidth / screenWidth;
        float Rh = emuHeight / screenHeight;

        float scaleX = Rw;
        float scaleY = Rh;

        // Debug: Print only window size
        static int lastW = 0, lastH = 0;
        if (screenWidth != lastW || screenHeight != lastH)
        {
            std::cout << " = = = = = = = { Window Info } = = = = = = = " << std::endl;
            std::cout << "Window: " << screenWidth << "x" << screenHeight << std::endl;
            std::cout << "Image: " << drawWidth << "x" << drawHeight << std::endl;
            std::cout << "EMULATED window: " << emuWidth << "x" << emuHeight << std::endl;
            std::cout << "Rw: " << Rw << " | " << Rh << std::endl;
            std::cout << "Scale: " << scaleX << " x " << scaleY << std::endl;
            std::cout << "border size: " << border << std::endl
                      << std::endl;
            lastW = screenWidth;
            lastH = screenHeight;
        }

        // --- Quad update and render ---
        auto updateAndRenderQuad = [&](float scaleX, float scaleY, GLuint VBO, GLuint VAO, GLuint program, GLuint texture)
        {
            updateQuadVBO(scaleX, scaleY, VBO);
            glClear(GL_COLOR_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUseProgram(program);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        };

        /*
        // Mouse input handling
        static bool justPressedLeftMouse = false;
        int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (mouseState == GLFW_PRESS && !justPressedLeftMouse)
        {
            double xpos, ypos;
            double normalizedX, normalizedY;
            int pixelX, pixelY;
            glfwGetCursorPos(window, &xpos, &ypos);

            normalizedX = ((xpos / screenWidth) - (1.0 - scaleX) / 2) / scaleX;
            normalizedY = ((ypos / screenHeight) - (1.0 - scaleY) / 2) / scaleY;

            // Clamp normalizedX and normalizedY to [0, 1]
            if (normalizedX < 0.0)
            normalizedX = 0.0;
            if (normalizedX > 1.0)
            normalizedX = 1.0;
            if (normalizedY < 0.0)
            normalizedY = 0.0;
            if (normalizedY > 1.0)
            normalizedY = 1.0;

            pixelX = mainImage->w * normalizedX;
            pixelY = mainImage->h * normalizedY;

            std::cout << " = = = = = = = { Mouse Info } = = = = = = = " << std::endl;
            std::cout << "Mouse just pressed at: (" << normalizedX << ", " << normalizedY << ")" << std::endl;
            std::cout << "Scale: " << scaleX << " x " << scaleY << std::endl;
            std::cout << "Ratio (Rw, Rh): " << Rw << ", " << Rh << std::endl;
            std::cout << "Image pos: " << pixelX << " : " << pixelY << std::endl
            << std::endl;
        }
        justPressedLeftMouse = (mouseState == GLFW_PRESS);
        */

        // ________________________________________________________________
        // --- Points editing and rendering ---
        static std::vector<float> points; // [x, y, x, y, ...] in NDC
        static std::vector<int> imgPoint; //
        static GLuint pointsVAO = 0, pointsVBO = 0;
        static bool pointsInitialized = false;

        auto editAndRenderPoints = [&]()
        {
            // Mouse input for adding points
            static bool justPressedLeftMouse = false;
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (mouseState == GLFW_PRESS && !justPressedLeftMouse)
            {
                double xpos, ypos;
                double normalizedX, normalizedY;
                int pixelX, pixelY;
                glfwGetCursorPos(window, &xpos, &ypos);

                // Convert to NDC
                float ndcX = 2.0f * ((float)xpos / screenWidth - 0.5f);
                float ndcY = 2.0f * (0.5f - (float)ypos / screenHeight);

                points.push_back(ndcX);
                points.push_back(ndcY);

                // - - - - To image coodinates - - - - //
                normalizedX = ((xpos / screenWidth) - (1.0 - scaleX) / 2) / scaleX;
                normalizedY = ((ypos / screenHeight) - (1.0 - scaleY) / 2) / scaleY;

                // Clamp normalizedX and normalizedY to [0, 1]
                if (normalizedX < 0.0)
                    normalizedX = 0.0;
                if (normalizedX > 1.0)
                    normalizedX = 1.0;
                if (normalizedY < 0.0)
                    normalizedY = 0.0;
                if (normalizedY > 1.0)
                    normalizedY = 1.0;

                pixelX = mainImage->w * normalizedX;
                pixelY = mainImage->h * normalizedY;

                imgPoint.push_back(pixelX);
                imgPoint.push_back(pixelY);

                // Update VBO
                if (!pointsInitialized)
                {
                    createPoints(points.data(), points.size() / 2, pointsVAO, pointsVBO);
                    pointsInitialized = true;
                }
                else
                {
                    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
                    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
                }
            }
            justPressedLeftMouse = (mouseState == GLFW_PRESS);

            // Render points
            if (pointsInitialized && !points.empty())
            {
                glPointSize(8.0f);
                glBindVertexArray(pointsVAO);
                glDrawArrays(GL_POINTS, 0, points.size() / 2);
            }

            // Check if '-' key was pressed to delete the latest point
            static bool justPressedMinus = false;
            int minusState = glfwGetKey(window, GLFW_KEY_KP_SUBTRACT);
            if (minusState != GLFW_PRESS)
            {
                minusState = glfwGetKey(window, GLFW_KEY_MINUS);
            }
            if (minusState == GLFW_PRESS && !justPressedMinus)
            {
                std::cout << "Pressed minus" << std::endl;

                if (!points.empty())
                {
                    points.pop_back();
                    points.pop_back();

                    imgPoint.pop_back();
                    imgPoint.pop_back();
                    // Update VBO
                    if (!points.empty())
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
                        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
                    }
                }
            }
            justPressedMinus = (minusState == GLFW_PRESS);
        };

        // _____________________________________________________________________________________________
        // .
        // Detect if Enter key was pressed
        static bool justPressedEnter = false;
        int enterState = glfwGetKey(window, GLFW_KEY_ENTER);
        if (enterState == GLFW_PRESS && !justPressedEnter)
        {
            stbi_set_flip_vertically_on_load(0);
            Image img(filename);
            img.write("output\\input.png");
            Image imageGradient = gradient::generateGradient(img);
            imageGradient.write("output\\gradient.png");

            std::map<int, int> seeds;
            // imgPoint contains [x0, y0, x1, y1, ...] in mainImage coordinates
            // We need to map them to gradientImage coordinates (may differ in size)
            for (size_t i = 0, label = 1; i + 1 < imgPoint.size(); i += 2, ++label)
            {
                int x = imgPoint[i];
                int y = imgPoint[i + 1];
                seeds[y * imageGradient.w + x] = static_cast<int>(label);
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

            EuclidianDistance_EdgeCost edgeCost(imageGradient); // Create an edge cost object

            CM cm(imageGradient, seeds, true); // Create a CM object with diagonal connections
            cm.edgeCost = &edgeCost;           // Set the edge cost function

            // _________________________________________________________________________________________________
            // Run the connected components algorithm

            cm.run(); // Run the connected components algorithm

            // _________________________________________________________________________________________________
            // Output

            Image outputImage(imageGradient.w, imageGradient.h, 3); // Create an output image with 3 channels (RGB)
            int lastLabel = -1;                                     // Initialize last label to -1
            for (int i = 0; i < imageGradient.w * imageGradient.h; ++i)
            {
                int label = cm.labels[i];

                if (label != -1) // If the pixel has a label
                {
                    auto color = labelColors[label]; // Get the color for the label

                    outputImage.data[i * 3] = std::get<0>(color);     // Set red channel
                    outputImage.data[i * 3 + 1] = std::get<1>(color); // Set green channel
                    outputImage.data[i * 3 + 2] = std::get<2>(color); // Set blue channel
                }
            }

            outputImage.write("output\\output.png"); // Write the output image to a file
            printf("Output image written to output\\output.png\n");
        }
        justPressedEnter = (enterState == GLFW_PRESS);

        // Handle right mouse button to open file dialog and change texture
        static bool justPressedRightMouse = false;
        int rightMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (rightMouseState == GLFW_PRESS && !justPressedRightMouse)
        {
            const char *filters[] = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
            filename = tinyfd_openFileDialog(
                "Open Image", "", 4, filters, "Image files", 0);
            if (filename)
            {

                GLuint newTexture = loadTexture(filename);
                if (newTexture)
                {
                    glDeleteTextures(1, &texture);
                    texture = newTexture;
                }
                else
                {
                    std::cerr << "Failed to load image: " << filename << std::endl;
                }
            }
        }
        justPressedRightMouse = (rightMouseState == GLFW_PRESS);

        // Render quad and points
        updateAndRenderQuad(scaleX, scaleY, VBO, VAO, program, texture);
        editAndRenderPoints();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main()
{

    // --- Main ---
    int screenWidth = 50, screenHeight = 50;
    GLFWwindow *window = initGLFW_GLAD(screenWidth, screenHeight, "Image Viewer");
    if (!window)
        return -1;

    GLuint texture = loadTexture("inhego.png");
    if (!texture)
    {
        std::cerr << "Failed to load image\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set desired border size in pixels
    float border = 0.1f;
    float imgAspect = (float)mainImage->w / mainImage->h;
    float drawWidth, drawHeight;
    float emuWidth, emuHeight;
    if (screenWidth < screenHeight)
    {
        drawWidth = screenWidth;
        drawHeight = drawWidth / imgAspect;

        emuWidth = screenWidth;
        emuHeight = emuWidth / imgAspect;
    }
    else
    {
        drawHeight = screenHeight;
        drawWidth = drawHeight * imgAspect;

        emuHeight = screenHeight;
        emuWidth = emuHeight * imgAspect;
    }

    float scaleX = (drawWidth / emuWidth) - border;
    float scaleY = (drawHeight / emuHeight) - border;
    GLuint VAO, VBO, EBO;
    createQuadVAO(scaleX, scaleY, VAO, VBO, EBO);

    GLuint program = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);
    GLuint points = createShaderProgram(pointVertexShaderSrc, pointFragmentShaderSrc);

    mainLoop(window, program, points, texture, VAO, VBO, border, "inhego.png");

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}