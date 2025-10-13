#pragma once

/*
 * CoordinateMapper - Centralized Coordinate Transformation
 *
 * Handles all coordinate system conversions:
 * - Screen (window) coordinates → Grid (simulation) coordinates
 * - Grid coordinates → Framebuffer coordinates
 * - Framebuffer coordinates → Window coordinates (for ImGui overlays)
 *
 * Coordinate Systems:
 * 1. Screen/Window: Top-left origin, Y-down (GLFW input coords)
 * 2. Framebuffer: Bottom-left origin, Y-up (OpenGL coords, may be DPI-scaled)
 * 3. Grid: Top-left origin, Y-down (simulation array indexing)
 *
 * This class eliminates duplicate conversion logic and provides a single
 * source of truth for coordinate transformations.
 */

class Renderer;  // Forward declaration

class CoordinateMapper {
public:
    CoordinateMapper();
    ~CoordinateMapper() = default;

    /*
     * Update viewport dimensions and DPI scaling
     *
     * Should be called whenever window is resized or renderer viewport changes
     *
     * @param winWidth Window width in screen coordinates
     * @param winHeight Window height in screen coordinates
     * @param fbWidth Framebuffer width in pixels (may differ due to DPI scaling)
     * @param fbHeight Framebuffer height in pixels
     * @param gridWidth Simulation grid width
     * @param gridHeight Simulation grid height
     * @param viewLeft Left edge of room viewport (framebuffer coords)
     * @param viewRight Right edge of room viewport (framebuffer coords)
     * @param viewBottom Bottom edge of room viewport (framebuffer coords)
     * @param viewTop Top edge of room viewport (framebuffer coords)
     */
    void updateViewport(
        int winWidth, int winHeight,
        int fbWidth, int fbHeight,
        int gridWidth, int gridHeight,
        float viewLeft, float viewRight,
        float viewBottom, float viewTop);

    /*
     * Convert screen coordinates to grid coordinates
     *
     * @param screenX Screen X coordinate (window coords, top-left origin)
     * @param screenY Screen Y coordinate (window coords, top-left origin)
     * @param[out] gridX Output grid X coordinate
     * @param[out] gridY Output grid Y coordinate
     * @return true if point is inside room viewport, false if outside
     */
    bool screenToGrid(double screenX, double screenY, int& gridX, int& gridY) const;

    /*
     * Convert grid coordinates to framebuffer coordinates
     *
     * @param gridX Grid X coordinate
     * @param gridY Grid Y coordinate
     * @param[out] fbX Output framebuffer X coordinate
     * @param[out] fbY Output framebuffer Y coordinate (bottom-up, Y-up)
     */
    void gridToFramebuffer(int gridX, int gridY, float& fbX, float& fbY) const;

    /*
     * Convert framebuffer coordinates to window coordinates (for ImGui)
     *
     * @param fbX Framebuffer X coordinate
     * @param fbY Framebuffer Y coordinate (bottom-up, Y-up)
     * @param[out] windowX Output window X coordinate (top-down, Y-down)
     * @param[out] windowY Output window Y coordinate (top-down, Y-down)
     */
    void framebufferToWindow(float fbX, float fbY, float& windowX, float& windowY) const;

    /*
     * Convert grid coordinates directly to window coordinates
     *
     * Convenience method that combines gridToFramebuffer + framebufferToWindow
     *
     * @param gridX Grid X coordinate
     * @param gridY Grid Y coordinate
     * @param[out] windowX Output window X coordinate
     * @param[out] windowY Output window Y coordinate
     */
    void gridToWindow(int gridX, int gridY, float& windowX, float& windowY) const;

private:
    // Window dimensions (screen coordinates)
    int windowWidth;
    int windowHeight;

    // Framebuffer dimensions (physical pixels, may differ from window due to DPI)
    int framebufferWidth;
    int framebufferHeight;

    // Grid dimensions (simulation resolution)
    int gridWidth;
    int gridHeight;

    // Room viewport bounds (in framebuffer coordinates)
    float viewportLeft;
    float viewportRight;
    float viewportBottom;
    float viewportTop;

    // DPI scaling factors
    float scaleX;  // framebufferWidth / windowWidth
    float scaleY;  // framebufferHeight / windowHeight
};
