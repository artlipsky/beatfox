#include "Application.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

Application::Application(int width, int height, const char* title)
    : window(nullptr)
    , windowWidth(width)
    , windowHeight(height)
    , windowTitle(title)
    , dpiScale(1.0f)
{
}

Application::~Application() {
    // Cleanup ImGui
    if (window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // Cleanup GLFW
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Application::initialize() {
    if (!initializeGLFW()) {
        return false;
    }

    if (!initializeOpenGL()) {
        return false;
    }

    if (!initializeImGui()) {
        return false;
    }

    return true;
}

bool Application::initializeGLFW() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set OpenGL version to 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    return true;
}

bool Application::initializeOpenGL() {
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

bool Application::initializeImGui() {
    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Get window content scale for DPI
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    dpiScale = xscale; // Use x scale (typically both are the same)

    std::cout << "DPI Scale: " << dpiScale << "x" << std::endl;

    // Setup Dear ImGui style - NO scaling, keep logical sizes
    ImGui::StyleColorsDark();

    // Load font with proper DPI scaling for sharpness, but reasonable size
    float baseFontSize = 14.0f;  // Logical size in points
    ImFontConfig fontConfig;
    fontConfig.SizePixels = baseFontSize * dpiScale;  // Physical pixels for sharpness
    fontConfig.RasterizerMultiply = 1.0f;
    io.Fonts->AddFontDefault(&fontConfig);
    io.FontGlobalScale = 1.0f / dpiScale;  // Scale back to logical size

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}
