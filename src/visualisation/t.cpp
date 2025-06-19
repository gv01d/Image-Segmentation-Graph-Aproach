// Render Image Centered with OpenGL + GLAD + GLFW
// Loads image using stb_image.h

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../image/image.cpp"
#include <iostream>

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

void mainLoop(GLFWwindow *window, GLuint program, GLuint texture, GLuint VAO, GLuint VBO, int imgWidth, int imgHeight, float border)
{
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "image"), 0);
    int screenWidth, screenHeight;
    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        float imgAspect = (float)imgWidth / imgHeight;
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

        float scaleX = Rw - border;
        float scaleY = Rh - border;

        // Debug: Print only window size
        /*
        static int lastW = 0, lastH = 0;
        if (screenWidth != lastW || screenHeight != lastH)
        {
            std::cout << "Window: " << screenWidth << "x" << screenHeight << std::endl;
            std::cout << "Image: " << drawWidth << "x" << drawHeight << std::endl;
            std::cout << "EMULATED window: " << emuWidth << "x" << emuHeight << std::endl;
            std::cout << "Rw: " << Rw << " | " << Rh << std::endl;
            std::cout << "Scale: " << scaleX << " x " << scaleY << std::endl;
            std::cout << "border size: " << border << std::endl;
            lastW = screenWidth;
            lastH = screenHeight;
        }
        */

        // Mouse input handling
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double xpos, ypos;
            double normalizedX, normalizedY;
            glfwGetCursorPos(window, &xpos, &ypos);

            normalizedX = ((xpos / screenWidth) - (1.0 - scaleX) / 2) / Rw;
            normalizedY = ((ypos / screenHeight) - (1.0 - scaleY) / 2) / Rh;

            std::cout << "Mouse pressed at: (" << normalizedX << ", " << normalizedY << ")" << std::endl;
        }

        updateQuadVBO(scaleX, scaleY, VBO);

        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

    mainLoop(window, program, texture, VAO, VBO, mainImage->w, mainImage->h, border);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}