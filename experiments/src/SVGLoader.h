#pragma once

#include <string>
#include <vector>
#include <memory>

/*
 * SVGLoader - Infrastructure layer component
 *
 * Loads SVG files and rasterizes them to an obstacle grid.
 * Uses nanosvg for parsing and rasterization.
 *
 * Design principles:
 * - Single Responsibility: Only handles SVG loading and rasterization
 * - Clean Architecture: Infrastructure layer component
 * - Domain isolation: Returns simple data structures (vectors) to domain
 */

// Forward declaration for nanosvg types
struct NSVGimage;
struct NSVGrasterizer;

class SVGLoader {
public:
    SVGLoader();
    ~SVGLoader();

    /*
     * Load SVG file and rasterize to obstacle grid
     *
     * @param filename Path to SVG file
     * @param gridWidth Output grid width in pixels
     * @param gridHeight Output grid height in pixels
     * @param obstacles Output vector (will be resized and filled)
     * @return true if successful, false on error
     *
     * SVG coordinate system is automatically mapped to grid dimensions.
     * Black/dark pixels in rasterized image become obstacles (1),
     * white/light pixels become empty space (0).
     */
    bool loadSVG(const std::string& filename,
                 int gridWidth,
                 int gridHeight,
                 std::vector<uint8_t>& obstacles);

    /*
     * Get last error message
     */
    const std::string& getLastError() const { return lastError; }

    /*
     * Get SVG dimensions (viewBox) after loading
     */
    float getSVGWidth() const { return svgWidth; }
    float getSVGHeight() const { return svgHeight; }

private:
    /*
     * Parse SVG file using nanosvg
     *
     * @param filename Path to SVG file
     * @param dpi DPI for unit conversion (96 is standard for web)
     * @return true if successful
     */
    bool parseSVG(const std::string& filename, float dpi = 96.0f);

    /*
     * Rasterize parsed SVG to pixel grid
     *
     * @param gridWidth Target grid width
     * @param gridHeight Target grid height
     * @param obstacles Output obstacle grid
     * @return true if successful
     */
    bool rasterizeSVG(int gridWidth,
                      int gridHeight,
                      std::vector<uint8_t>& obstacles);

    /*
     * Convert RGBA image buffer to binary obstacle map
     *
     * Pixels with alpha > 0 and low luminance become obstacles.
     * This allows SVG strokes and fills to define walls.
     *
     * @param rgbaBuffer RGBA image buffer (width * height * 4 bytes)
     * @param width Image width
     * @param height Image height
     * @param obstacles Output obstacle grid
     */
    void convertToObstacles(const uint8_t* rgbaBuffer,
                           int width,
                           int height,
                           std::vector<uint8_t>& obstacles);

    NSVGimage* svgImage;           // Parsed SVG image
    NSVGrasterizer* rasterizer;    // nanosvg rasterizer

    float svgWidth;                // SVG viewBox width
    float svgHeight;               // SVG viewBox height

    std::string lastError;         // Last error message
};
