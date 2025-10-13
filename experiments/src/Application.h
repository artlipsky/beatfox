#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application {
public:
    // Constructor takes initial window dimensions
    Application(int width, int height, const char* title);

    // Destructor handles cleanup
    ~Application();

    // Initialize GLFW, OpenGL/GLAD, and ImGui
    bool initialize();

    // Accessors
    GLFWwindow* getWindow() const { return window; }
    float getDpiScale() const { return dpiScale; }
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

private:
    // Window management
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    const char* windowTitle;

    // DPI scaling
    float dpiScale;

    // Initialization helpers
    bool initializeGLFW();
    bool initializeOpenGL();
    bool initializeImGui();
};

#endif // APPLICATION_H
