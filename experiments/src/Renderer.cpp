#include "Renderer.h"
#include "WaveSimulation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

// GLAD loader implementation - we'll include glad.c in main.cpp

Renderer::Renderer(int windowWidth, int windowHeight)
    : windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , roomViewportX(0)
    , roomViewportY(0)
    , roomViewportWidth(0)
    , roomViewportHeight(0)
    , shaderProgram(0)
    , VAO(0)
    , VBO(0)
    , EBO(0)
    , projectionLoc(0)
{
    calculateRoomViewport();
}

Renderer::~Renderer() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);

    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);
    if (gridShaderProgram) glDeleteProgram(gridShaderProgram);
}

bool Renderer::initialize() {
    if (!loadShaders()) {
        std::cerr << "Failed to load shaders" << std::endl;
        return false;
    }

    glUseProgram(shaderProgram);
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Load grid shaders
    if (!loadGridShaders()) {
        std::cerr << "Failed to load grid shaders" << std::endl;
        return false;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Renderer::setupBuffers(int gridWidth, int gridHeight) {
    // Store grid dimensions for aspect ratio calculation
    this->gridWidth = gridWidth;
    this->gridHeight = gridHeight;

    // Recalculate viewport with new aspect ratio
    calculateRoomViewport();

    // Clean up old buffers if they exist
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    vertices.clear();
    indices.clear();

    // Create vertex data (position + height + obstacle)
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            // Normalized coordinates [-1, 1]
            float px = 2.0f * x / (gridWidth - 1) - 1.0f;
            float py = 2.0f * y / (gridHeight - 1) - 1.0f;

            vertices.push_back(px);
            vertices.push_back(py);
            vertices.push_back(0.0f); // Height placeholder
            vertices.push_back(0.0f); // Obstacle flag placeholder
        }
    }

    // Create indices for triangle strips
    for (int y = 0; y < gridHeight - 1; y++) {
        for (int x = 0; x < gridWidth - 1; x++) {
            int topLeft = y * gridWidth + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * gridWidth + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Create and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // Create and bind EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Height attribute (location = 1)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Obstacle attribute (location = 2)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Renderer::render(const WaveSimulation& simulation) {
    int gridWidth = simulation.getWidth();
    int gridHeight = simulation.getHeight();

    // Setup buffers if not already done
    if (VAO == 0) {
        setupBuffers(gridWidth, gridHeight);
    }

    // Update height and obstacle values in vertex buffer
    const float* waveData = simulation.getData();
    const uint8_t* obstacleData = simulation.getObstacles();
    for (int i = 0; i < gridWidth * gridHeight; i++) {
        vertices[i * 4 + 2] = waveData[i];           // Height
        vertices[i * 4 + 3] = obstacleData[i] ? 1.0f : 0.0f;  // Obstacle flag
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    // Clear entire screen with dark background (outside room)
    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set viewport to the centered room area
    glViewport(static_cast<int>(roomViewportX),
               static_cast<int>(roomViewportY),
               static_cast<int>(roomViewportWidth),
               static_cast<int>(roomViewportHeight));

    // Use shader and set uniforms
    glUseProgram(shaderProgram);

    // Simple orthographic projection for the room [-1, 1] x [-1, 1]
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Draw wave simulation
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Draw grid overlay
    if (gridEnabled) {
        renderGrid(gridWidth, gridHeight);
    }

    // Reset viewport to full window for UI rendering
    glViewport(0, 0, windowWidth, windowHeight);
}

void Renderer::resize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    calculateRoomViewport();
}

void Renderer::calculateRoomViewport() {
    // Calculate viewport to center the room with padding
    // Calculate aspect ratio dynamically based on grid dimensions
    float roomAspect = (gridWidth > 0 && gridHeight > 0)
        ? static_cast<float>(gridWidth) / static_cast<float>(gridHeight)
        : 2.0f;  // Default fallback

    // Available space after padding
    float availableWidth = windowWidth - 2 * padding;
    float availableHeight = windowHeight - 2 * padding;
    float availableAspect = availableWidth / availableHeight;

    if (availableAspect > roomAspect) {
        // Window is wider than needed - fit to height
        roomViewportHeight = availableHeight;
        roomViewportWidth = roomViewportHeight * roomAspect;
    } else {
        // Window is taller than needed - fit to width
        roomViewportWidth = availableWidth;
        roomViewportHeight = roomViewportWidth / roomAspect;
    }

    // Center the viewport
    roomViewportX = (windowWidth - roomViewportWidth) / 2.0f;
    roomViewportY = (windowHeight - roomViewportHeight) / 2.0f;
}

void Renderer::getRoomViewport(float& left, float& right, float& bottom, float& top) const {
    left = roomViewportX;
    right = roomViewportX + roomViewportWidth;
    bottom = roomViewportY;
    top = roomViewportY + roomViewportHeight;
}

void Renderer::updateGridDimensions(int width, int height) {
    gridWidth = width;
    gridHeight = height;
    calculateRoomViewport();
}

GLuint Renderer::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return 0;
    }

    return shader;
}

std::string Renderer::loadShaderFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Renderer::loadShaders() {
    std::string vertexSource = loadShaderFile("shaders/wave.vert");
    std::string fragmentSource = loadShaderFile("shaders/wave.frag");

    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vertexShader || !fragmentShader) {
        return false;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

bool Renderer::loadGridShaders() {
    // Vertex shader for grid lines with major/minor attribute
    const char* gridVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in float aMajor;
        uniform mat4 projection;
        out float isMajor;
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
            isMajor = aMajor;
        }
    )";

    // Fragment shader for grid lines with major/minor distinction
    const char* gridFragmentSource = R"(
        #version 330 core
        in float isMajor;
        out vec4 FragColor;
        void main() {
            if (isMajor > 0.5) {
                // Major lines: bright white, more opaque
                FragColor = vec4(1.0, 1.0, 1.0, 0.35);
            } else {
                // Minor lines: subtle gray, low opacity
                FragColor = vec4(0.6, 0.6, 0.6, 0.12);
            }
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, gridVertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, gridFragmentSource);

    if (!vertexShader || !fragmentShader) {
        return false;
    }

    gridShaderProgram = glCreateProgram();
    glAttachShader(gridShaderProgram, vertexShader);
    glAttachShader(gridShaderProgram, fragmentShader);
    glLinkProgram(gridShaderProgram);

    int success;
    glGetProgramiv(gridShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(gridShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Grid shader linking failed: " << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get uniform locations
    gridProjectionLoc = glGetUniformLocation(gridShaderProgram, "projection");

    return true;
}

void Renderer::setupGridBuffers(int gridWidth, int gridHeight) {
    // Clean up old buffers if they exist
    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);

    std::vector<float> gridVertices;  // Format: x, y, isMajor

    // Generate vertical grid lines (along X axis)
    for (int x = 0; x <= gridWidth; x += gridSpacing) {
        float px = 2.0f * x / (gridWidth - 1) - 1.0f;

        // Determine if this is a major grid line (every 10th line)
        float isMajor = (x % (gridSpacing * 10) == 0) ? 1.0f : 0.0f;

        // Line from bottom to top (2 vertices per line)
        gridVertices.push_back(px);       // x1
        gridVertices.push_back(-1.0f);    // y1
        gridVertices.push_back(isMajor);  // major flag
        gridVertices.push_back(px);       // x2
        gridVertices.push_back(1.0f);     // y2
        gridVertices.push_back(isMajor);  // major flag
    }

    // Generate horizontal grid lines (along Y axis)
    for (int y = 0; y <= gridHeight; y += gridSpacing) {
        float py = 2.0f * y / (gridHeight - 1) - 1.0f;

        // Determine if this is a major grid line (every 10th line)
        float isMajor = (y % (gridSpacing * 10) == 0) ? 1.0f : 0.0f;

        // Line from left to right (2 vertices per line)
        gridVertices.push_back(-1.0f);    // x1
        gridVertices.push_back(py);       // y1
        gridVertices.push_back(isMajor);  // major flag
        gridVertices.push_back(1.0f);     // x2
        gridVertices.push_back(py);       // y2
        gridVertices.push_back(isMajor);  // major flag
    }

    gridLineCount = gridVertices.size() / 3;  // Total vertices (3 floats per vertex)

    // Create and bind VAO
    glGenVertexArrays(1, &gridVAO);
    glBindVertexArray(gridVAO);

    // Create and bind VBO
    glGenBuffers(1, &gridVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Major flag attribute (location = 1)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Renderer::renderGrid(int gridWidth, int gridHeight) {
    // Setup grid buffers if not already done
    if (gridVAO == 0) {
        setupGridBuffers(gridWidth, gridHeight);
    }

    // Use grid shader
    glUseProgram(gridShaderProgram);

    // Set projection matrix
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(gridProjectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind VAO and draw all grid lines
    // The shader will distinguish major/minor based on the vertex attribute
    glBindVertexArray(gridVAO);
    glDrawArrays(0x0001, 0, gridLineCount);  // 0x0001 = GL_LINES
    glBindVertexArray(0);
}
