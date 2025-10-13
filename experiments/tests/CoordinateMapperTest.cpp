#include <gtest/gtest.h>
#include "../src/CoordinateMapper.h"

/*
 * CoordinateMapper Unit Tests
 *
 * Tests all coordinate transformation methods with various scenarios:
 * - Normal DPI (1x scaling)
 * - High DPI (2x scaling - typical Retina displays)
 * - Different grid sizes
 * - Boundary conditions
 * - Out-of-bounds handling
 */

class CoordinateMapperTest : public ::testing::Test {
protected:
    CoordinateMapper mapper;

    void SetUp() override {
        // Default setup: 1280x720 window, 2560x1440 framebuffer (2x DPI)
        // Grid: 400x200, Viewport: (40, 100) to (2520, 1340)
        mapper.updateViewport(
            1280, 720,      // Window dimensions
            2560, 1440,     // Framebuffer dimensions (2x DPI)
            400, 200,       // Grid dimensions
            40.0f, 2520.0f, // Viewport left, right
            100.0f, 1340.0f // Viewport bottom, top
        );
    }
};

// ============================================================================
// Screen to Grid Conversion Tests
// ============================================================================

TEST_F(CoordinateMapperTest, ScreenToGrid_CenterPoint) {
    // Center of window should map to center of grid
    int gridX, gridY;
    bool result = mapper.screenToGrid(640.0, 360.0, gridX, gridY);

    EXPECT_TRUE(result);
    EXPECT_NEAR(gridX, 200, 1);  // Center X (±1 pixel tolerance)
    EXPECT_NEAR(gridY, 100, 1);  // Center Y
}

TEST_F(CoordinateMapperTest, ScreenToGrid_TopLeftCorner) {
    // Top-left corner of viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(20.0, 50.0, gridX, gridY);

    EXPECT_TRUE(result);
    EXPECT_EQ(gridX, 0);
    EXPECT_EQ(gridY, 0);
}

TEST_F(CoordinateMapperTest, ScreenToGrid_BottomRightCorner) {
    // Bottom-right corner of viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(1260.0, 670.0, gridX, gridY);

    EXPECT_TRUE(result);
    EXPECT_EQ(gridX, 399);  // Last valid grid X
    EXPECT_EQ(gridY, 199);  // Last valid grid Y
}

TEST_F(CoordinateMapperTest, ScreenToGrid_OutOfBounds_Left) {
    // Click to the left of viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(10.0, 360.0, gridX, gridY);

    EXPECT_FALSE(result);  // Should return false for out-of-bounds
}

TEST_F(CoordinateMapperTest, ScreenToGrid_OutOfBounds_Right) {
    // Click to the right of viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(1270.0, 360.0, gridX, gridY);

    EXPECT_FALSE(result);
}

TEST_F(CoordinateMapperTest, ScreenToGrid_OutOfBounds_Top) {
    // Click above viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(640.0, 40.0, gridX, gridY);

    EXPECT_FALSE(result);
}

TEST_F(CoordinateMapperTest, ScreenToGrid_OutOfBounds_Bottom) {
    // Click below viewport
    int gridX, gridY;
    bool result = mapper.screenToGrid(640.0, 680.0, gridX, gridY);

    EXPECT_FALSE(result);
}

TEST_F(CoordinateMapperTest, ScreenToGrid_Clamping) {
    // Values slightly outside should be clamped to valid range
    // This tests the std::max/std::min clamping logic
    int gridX, gridY;

    // Just inside top-left
    bool result = mapper.screenToGrid(21.0, 51.0, gridX, gridY);
    EXPECT_TRUE(result);
    EXPECT_GE(gridX, 0);
    EXPECT_GE(gridY, 0);
    EXPECT_LT(gridX, 400);
    EXPECT_LT(gridY, 200);
}

// ============================================================================
// Grid to Framebuffer Conversion Tests
// ============================================================================

TEST_F(CoordinateMapperTest, GridToFramebuffer_Origin) {
    // Grid origin (0, 0) should map to viewport top-left
    // Grid Y=0 is top, so fbY should be high (viewportTop)
    float fbX, fbY;
    mapper.gridToFramebuffer(0, 0, fbX, fbY);

    EXPECT_FLOAT_EQ(fbX, 40.0f);   // viewportLeft
    EXPECT_FLOAT_EQ(fbY, 1340.0f);  // viewportTop (top of viewport in Y-up coords)
}

TEST_F(CoordinateMapperTest, GridToFramebuffer_MaxCorner) {
    // Grid max corner (399, 199) should map to viewport bottom-right
    // Grid Y=199 is bottom, so fbY should be low (viewportBottom)
    float fbX, fbY;
    mapper.gridToFramebuffer(399, 199, fbX, fbY);

    EXPECT_NEAR(fbX, 2520.0f, 10.0f);  // viewportRight (±10 pixel tolerance)
    EXPECT_NEAR(fbY, 100.0f, 10.0f);   // viewportBottom (bottom of viewport in Y-up coords)
}

TEST_F(CoordinateMapperTest, GridToFramebuffer_Center) {
    // Grid center should map to viewport center
    float fbX, fbY;
    mapper.gridToFramebuffer(200, 100, fbX, fbY);

    float expectedX = (40.0f + 2520.0f) / 2.0f;  // Viewport center X
    float expectedY = (100.0f + 1340.0f) / 2.0f; // Viewport center Y

    EXPECT_NEAR(fbX, expectedX, 5.0f);
    EXPECT_NEAR(fbY, expectedY, 5.0f);
}

// ============================================================================
// Framebuffer to Window Conversion Tests
// ============================================================================

TEST_F(CoordinateMapperTest, FramebufferToWindow_Origin) {
    // Framebuffer origin (bottom-left in OpenGL)
    float winX, winY;
    mapper.framebufferToWindow(0.0f, 0.0f, winX, winY);

    EXPECT_FLOAT_EQ(winX, 0.0f);
    EXPECT_FLOAT_EQ(winY, 720.0f);  // Bottom of framebuffer = bottom of window
}

TEST_F(CoordinateMapperTest, FramebufferToWindow_TopRight) {
    // Framebuffer top-right
    float winX, winY;
    mapper.framebufferToWindow(2560.0f, 1440.0f, winX, winY);

    EXPECT_FLOAT_EQ(winX, 1280.0f);  // Right edge of window
    EXPECT_FLOAT_EQ(winY, 0.0f);     // Top of window (Y-down)
}

TEST_F(CoordinateMapperTest, FramebufferToWindow_Center) {
    // Framebuffer center
    float winX, winY;
    mapper.framebufferToWindow(1280.0f, 720.0f, winX, winY);

    EXPECT_FLOAT_EQ(winX, 640.0f);   // Window center X
    EXPECT_FLOAT_EQ(winY, 360.0f);   // Window center Y
}

// ============================================================================
// Combined Grid to Window Conversion Tests
// ============================================================================

TEST_F(CoordinateMapperTest, GridToWindow_RoundTrip) {
    // Test that grid → window → screen → grid preserves coordinates
    int originalGridX = 100;
    int originalGridY = 50;

    // Grid → Window
    float winX, winY;
    mapper.gridToWindow(originalGridX, originalGridY, winX, winY);

    // Window → Grid (via screenToGrid which expects screen coords)
    int recoveredGridX, recoveredGridY;
    bool result = mapper.screenToGrid(winX, winY, recoveredGridX, recoveredGridY);

    EXPECT_TRUE(result);
    EXPECT_NEAR(recoveredGridX, originalGridX, 2);  // ±2 pixel tolerance for rounding
    EXPECT_NEAR(recoveredGridY, originalGridY, 2);
}

TEST_F(CoordinateMapperTest, GridToWindow_CenterPoint) {
    // Grid center should map to window center
    float winX, winY;
    mapper.gridToWindow(200, 100, winX, winY);

    EXPECT_NEAR(winX, 640.0f, 5.0f);   // Window center X
    EXPECT_NEAR(winY, 360.0f, 5.0f);   // Window center Y
}

// ============================================================================
// DPI Scaling Tests
// ============================================================================

TEST_F(CoordinateMapperTest, DPIScaling_1x) {
    // Test with 1x DPI (no scaling)
    CoordinateMapper mapper1x;
    mapper1x.updateViewport(
        800, 600,       // Window = Framebuffer (1x DPI)
        800, 600,
        400, 200,
        0.0f, 800.0f,
        0.0f, 600.0f
    );

    int gridX, gridY;
    bool result = mapper1x.screenToGrid(400.0, 300.0, gridX, gridY);  // Center

    EXPECT_TRUE(result);
    EXPECT_NEAR(gridX, 200, 1);  // Grid center X
    EXPECT_NEAR(gridY, 100, 1);  // Grid center Y
}

TEST_F(CoordinateMapperTest, DPIScaling_3x) {
    // Test with 3x DPI (high-density display)
    CoordinateMapper mapper3x;
    mapper3x.updateViewport(
        800, 600,           // Window dimensions
        2400, 1800,         // Framebuffer dimensions (3x)
        400, 200,
        0.0f, 2400.0f,
        0.0f, 1800.0f
    );

    int gridX, gridY;
    bool result = mapper3x.screenToGrid(400.0, 300.0, gridX, gridY);  // Center

    EXPECT_TRUE(result);
    EXPECT_NEAR(gridX, 200, 1);  // Grid center X
    EXPECT_NEAR(gridY, 100, 1);  // Grid center Y
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(CoordinateMapperTest, ZeroDimensions) {
    // Test with zero dimensions (should not crash)
    CoordinateMapper mapperZero;
    mapperZero.updateViewport(0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f);

    int gridX, gridY;
    bool result = mapperZero.screenToGrid(0.0, 0.0, gridX, gridY);

    // Should handle gracefully without crashing (behavior is defined)
    // We don't strictly require false, just that it doesn't crash
    SUCCEED() << "Zero dimensions handled without crash";
}

TEST_F(CoordinateMapperTest, NegativeCoordinates) {
    // Test with negative screen coordinates
    int gridX, gridY;
    bool result = mapper.screenToGrid(-10.0, -10.0, gridX, gridY);

    EXPECT_FALSE(result);  // Negative coords are outside viewport
}

TEST_F(CoordinateMapperTest, LargeCoordinates) {
    // Test with very large coordinates
    int gridX, gridY;
    bool result = mapper.screenToGrid(10000.0, 10000.0, gridX, gridY);

    EXPECT_FALSE(result);  // Far outside viewport
}

// ============================================================================
// Different Grid Size Tests
// ============================================================================

TEST_F(CoordinateMapperTest, SmallGrid) {
    // Test with very small grid (edge case)
    CoordinateMapper smallMapper;
    smallMapper.updateViewport(
        800, 600,
        800, 600,
        10, 10,         // Very small 10x10 grid
        0.0f, 800.0f,
        0.0f, 600.0f
    );

    int gridX, gridY;
    bool result = smallMapper.screenToGrid(400.0, 300.0, gridX, gridY);

    EXPECT_TRUE(result);
    EXPECT_GE(gridX, 0);
    EXPECT_LT(gridX, 10);
    EXPECT_GE(gridY, 0);
    EXPECT_LT(gridY, 10);
}

TEST_F(CoordinateMapperTest, LargeGrid) {
    // Test with large grid
    CoordinateMapper largeMapper;
    largeMapper.updateViewport(
        800, 600,
        800, 600,
        2000, 1500,     // Large 2000x1500 grid
        0.0f, 800.0f,
        0.0f, 600.0f
    );

    int gridX, gridY;
    bool result = largeMapper.screenToGrid(400.0, 300.0, gridX, gridY);

    EXPECT_TRUE(result);
    EXPECT_NEAR(gridX, 1000, 10);  // Grid center X (±10 tolerance for large grid)
    EXPECT_NEAR(gridY, 750, 10);   // Grid center Y
}
