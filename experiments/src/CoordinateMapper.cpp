#include "CoordinateMapper.h"
#include <algorithm>

CoordinateMapper::CoordinateMapper()
    : windowWidth(0)
    , windowHeight(0)
    , framebufferWidth(0)
    , framebufferHeight(0)
    , gridWidth(0)
    , gridHeight(0)
    , viewportLeft(0.0f)
    , viewportRight(0.0f)
    , viewportBottom(0.0f)
    , viewportTop(0.0f)
    , scaleX(1.0f)
    , scaleY(1.0f)
{
}

void CoordinateMapper::updateViewport(
    int winWidth, int winHeight,
    int fbWidth, int fbHeight,
    int gWidth, int gHeight,
    float viewLeft, float viewRight,
    float viewBottom, float viewTop)
{
    windowWidth = winWidth;
    windowHeight = winHeight;
    framebufferWidth = fbWidth;
    framebufferHeight = fbHeight;
    gridWidth = gWidth;
    gridHeight = gHeight;
    viewportLeft = viewLeft;
    viewportRight = viewRight;
    viewportBottom = viewBottom;
    viewportTop = viewTop;

    // Calculate DPI scaling factors
    scaleX = (winWidth > 0) ? static_cast<float>(fbWidth) / static_cast<float>(winWidth) : 1.0f;
    scaleY = (winHeight > 0) ? static_cast<float>(fbHeight) / static_cast<float>(winHeight) : 1.0f;
}

bool CoordinateMapper::screenToGrid(double screenX, double screenY, int& gridX, int& gridY) const {
    // Convert window coordinates to framebuffer coordinates
    float fbX = static_cast<float>(screenX) * scaleX;
    float fbY = static_cast<float>(screenY) * scaleY;

    // Framebuffer Y is bottom-up, but input Y is top-down - flip it
    float fbYFlipped = framebufferHeight - fbY;

    // Check if click is inside the room viewport
    if (fbX < viewportLeft || fbX > viewportRight ||
        fbYFlipped < viewportBottom || fbYFlipped > viewportTop) {
        return false;  // Click outside room
    }

    // Map from room viewport to simulation grid
    float viewportWidth = viewportRight - viewportLeft;
    float viewportHeight = viewportTop - viewportBottom;

    float normalizedX = (fbX - viewportLeft) / viewportWidth;
    // Map viewport to grid: bottom of viewport → gridY=0, top → gridY=height-1
    float normalizedY = (fbYFlipped - viewportBottom) / viewportHeight;

    gridX = static_cast<int>(normalizedX * gridWidth);
    gridY = static_cast<int>(normalizedY * gridHeight);

    // Clamp to valid range
    gridX = std::max(0, std::min(gridX, gridWidth - 1));
    gridY = std::max(0, std::min(gridY, gridHeight - 1));

    return true;  // Valid click inside room
}

void CoordinateMapper::gridToFramebuffer(int gridX, int gridY, float& fbX, float& fbY) const {
    // Map grid coordinates to framebuffer coordinates (bottom-up Y)
    float normalizedX = static_cast<float>(gridX) / static_cast<float>(gridWidth);
    float normalizedY = static_cast<float>(gridY) / static_cast<float>(gridHeight);

    float viewportWidth = viewportRight - viewportLeft;
    float viewportHeight = viewportTop - viewportBottom;

    fbX = viewportLeft + normalizedX * viewportWidth;
    // Map grid to viewport: gridY=0 → low fbY (viewBottom), gridY=height-1 → high fbY (viewTop)
    fbY = viewportBottom + normalizedY * viewportHeight;
}

void CoordinateMapper::framebufferToWindow(float fbX, float fbY, float& windowX, float& windowY) const {
    // Convert from framebuffer coordinates to window coordinates
    // ImGui uses window coordinates (top-down Y)
    windowX = fbX / scaleX;
    windowY = (framebufferHeight - fbY) / scaleY;  // Flip Y axis
}

void CoordinateMapper::gridToWindow(int gridX, int gridY, float& windowX, float& windowY) const {
    // Combine grid → framebuffer → window transformations
    float fbX, fbY;
    gridToFramebuffer(gridX, gridY, fbX, fbY);
    framebufferToWindow(fbX, fbY, windowX, windowY);
}
