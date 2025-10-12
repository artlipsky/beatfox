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

    bool loadShaders();
    GLuint compileShader(GLenum type, const std::string& source);
    std::string loadShaderFile(const std::string& filepath);
    void setupBuffers(int gridWidth, int gridHeight);
    void calculateRoomViewport();
};