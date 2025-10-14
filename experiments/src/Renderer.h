#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class WaveSimulation;

class Renderer {
public:
    Renderer(int windowWidth, int windowHeight);
    ~Renderer();

    bool initialize();
    void render(const WaveSimulation& simulation);
    void resize(int width, int height);

    // Get room viewport (for mouse coordinate mapping)
    void getRoomViewport(float& left, float& right, float& bottom, float& top) const;

    // Grid configuration
    void setGridEnabled(bool enabled) { gridEnabled = enabled; }
    bool isGridEnabled() const { return gridEnabled; }
    void setGridSpacing(int spacing) { gridSpacing = spacing; }

private:
    int windowWidth;
    int windowHeight;

    // Room viewport with padding
    float padding = 40.0f;  // Padding in pixels around the room
    float roomViewportX, roomViewportY;      // Bottom-left corner
    float roomViewportWidth, roomViewportHeight;  // Size in pixels

    GLuint shaderProgram;
    GLuint VAO, VBO, EBO;
    GLuint projectionLoc;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Grid rendering
    bool gridEnabled = true;
    int gridSpacing = 10;  // Grid line every N simulation cells
    GLuint gridVAO = 0;
    GLuint gridVBO = 0;
    GLuint gridShaderProgram = 0;
    GLuint gridProjectionLoc = 0;

    bool loadShaders();
    GLuint compileShader(GLenum type, const std::string& source);
    std::string loadShaderFile(const std::string& filepath);
    void setupBuffers(int gridWidth, int gridHeight);
    void calculateRoomViewport();
    void renderGrid(int gridWidth, int gridHeight);
    bool loadGridShaders();
    void setupGridBuffers(int gridWidth, int gridHeight);
};