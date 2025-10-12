#include "SVGLoader.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// nanosvg - single-header SVG parser and rasterizer
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

SVGLoader::SVGLoader()
    : svgImage(nullptr)
    , rasterizer(nullptr)
    , svgWidth(0.0f)
    , svgHeight(0.0f)
{
    // Create rasterizer (can be reused for multiple images)
    rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        lastError = "Failed to create SVG rasterizer";
        std::cerr << "SVGLoader: " << lastError << std::endl;
    }
}

SVGLoader::~SVGLoader() {
    if (svgImage) {
        nsvgDelete(svgImage);
        svgImage = nullptr;
    }
    if (rasterizer) {
        nsvgDeleteRasterizer(rasterizer);
        rasterizer = nullptr;
    }
}

bool SVGLoader::loadSVG(const std::string& filename,
                        int gridWidth,
                        int gridHeight,
                        std::vector<uint8_t>& obstacles) {
    lastError.clear();

    // Step 1: Parse SVG file
    if (!parseSVG(filename)) {
        return false;
    }

    // Step 2: Rasterize to obstacle grid
    if (!rasterizeSVG(gridWidth, gridHeight, obstacles)) {
        return false;
    }

    std::cout << "SVGLoader: Successfully loaded " << filename << std::endl;
    std::cout << "  SVG dimensions: " << svgWidth << " x " << svgHeight << std::endl;
    std::cout << "  Grid dimensions: " << gridWidth << " x " << gridHeight << std::endl;

    return true;
}

bool SVGLoader::parseSVG(const std::string& filename, float dpi) {
    // Free previous image if any
    if (svgImage) {
        nsvgDelete(svgImage);
        svgImage = nullptr;
    }

    // Parse SVG file
    // Units: "px" means pixels (default), but SVG can use "pt", "pc", "mm", "cm", "in"
    // dpi is used to convert these units to pixels
    svgImage = nsvgParseFromFile(filename.c_str(), "px", dpi);

    if (!svgImage) {
        lastError = "Failed to parse SVG file: " + filename;
        std::cerr << "SVGLoader: " << lastError << std::endl;
        return false;
    }

    // Get SVG dimensions from viewBox
    svgWidth = svgImage->width;
    svgHeight = svgImage->height;

    if (svgWidth <= 0.0f || svgHeight <= 0.0f) {
        lastError = "Invalid SVG dimensions: " +
                   std::to_string(svgWidth) + " x " + std::to_string(svgHeight);
        std::cerr << "SVGLoader: " << lastError << std::endl;
        nsvgDelete(svgImage);
        svgImage = nullptr;
        return false;
    }

    // Count shapes for debugging
    int shapeCount = 0;
    for (NSVGshape* shape = svgImage->shapes; shape != nullptr; shape = shape->next) {
        shapeCount++;
    }

    std::cout << "SVGLoader: Parsed " << shapeCount << " shapes from " << filename << std::endl;

    return true;
}

bool SVGLoader::rasterizeSVG(int gridWidth,
                             int gridHeight,
                             std::vector<uint8_t>& obstacles) {
    if (!svgImage || !rasterizer) {
        lastError = "No SVG image loaded or rasterizer not initialized";
        std::cerr << "SVGLoader: " << lastError << std::endl;
        return false;
    }

    // Calculate scale to fit SVG into grid while preserving aspect ratio
    float scaleX = static_cast<float>(gridWidth) / svgWidth;
    float scaleY = static_cast<float>(gridHeight) / svgHeight;
    float scale = std::min(scaleX, scaleY);

    // Calculate actual rasterized dimensions
    int rasterWidth = static_cast<int>(svgWidth * scale);
    int rasterHeight = static_cast<int>(svgHeight * scale);

    // Ensure at least 1 pixel
    rasterWidth = std::max(1, rasterWidth);
    rasterHeight = std::max(1, rasterHeight);

    // Allocate RGBA buffer (4 bytes per pixel)
    std::vector<uint8_t> rgbaBuffer(rasterWidth * rasterHeight * 4, 0);

    // Rasterize SVG to RGBA buffer
    // nanosvg uses premultiplied alpha and outputs RGBA in ABGR format on little-endian
    nsvgRasterize(rasterizer,
                  svgImage,
                  0, 0,              // offset x, y
                  scale,             // scale
                  rgbaBuffer.data(), // output buffer
                  rasterWidth,
                  rasterHeight,
                  rasterWidth * 4);  // stride in bytes

    std::cout << "SVGLoader: Rasterized to " << rasterWidth << " x " << rasterHeight
              << " (scale: " << scale << ")" << std::endl;

    // Convert RGBA buffer to obstacle grid
    convertToObstacles(rgbaBuffer.data(), rasterWidth, rasterHeight, obstacles);

    // Resize and pad if rasterized size doesn't match grid size
    if (rasterWidth != gridWidth || rasterHeight != gridHeight) {
        std::vector<uint8_t> paddedObstacles(gridWidth * gridHeight, 0);

        // Center the rasterized image in the grid
        int offsetX = (gridWidth - rasterWidth) / 2;
        int offsetY = (gridHeight - rasterHeight) / 2;

        for (int y = 0; y < rasterHeight; y++) {
            for (int x = 0; x < rasterWidth; x++) {
                int srcIdx = y * rasterWidth + x;
                int dstX = x + offsetX;
                int dstY = y + offsetY;

                if (dstX >= 0 && dstX < gridWidth && dstY >= 0 && dstY < gridHeight) {
                    int dstIdx = dstY * gridWidth + dstX;
                    paddedObstacles[dstIdx] = obstacles[srcIdx];
                }
            }
        }

        obstacles = std::move(paddedObstacles);
    }

    return true;
}

void SVGLoader::convertToObstacles(const uint8_t* rgbaBuffer,
                                   int width,
                                   int height,
                                   std::vector<uint8_t>& obstacles) {
    obstacles.resize(width * height);

    /*
     * Convert RGBA pixels to binary obstacle map
     *
     * Strategy:
     * - nanosvg outputs RGBA with premultiplied alpha
     * - Black/dark pixels with alpha > 0 become obstacles
     * - Transparent pixels (alpha == 0) become empty space
     * - Use luminance threshold to distinguish walls from empty space
     *
     * Luminance formula (ITU-R BT.709):
     * Y = 0.2126*R + 0.7152*G + 0.0722*B
     */

    const float luminanceThreshold = 0.5f; // 50% gray threshold

    int obstacleCount = 0;

    for (int i = 0; i < width * height; i++) {
        // nanosvg outputs RGBA (on little-endian: ABGR in memory)
        uint8_t r = rgbaBuffer[i * 4 + 0];
        uint8_t g = rgbaBuffer[i * 4 + 1];
        uint8_t b = rgbaBuffer[i * 4 + 2];
        uint8_t a = rgbaBuffer[i * 4 + 3];

        // If pixel has alpha (is visible)
        if (a > 0) {
            // Calculate normalized luminance [0, 1]
            float luminance = (0.2126f * r + 0.7152f * g + 0.0722f * b) / 255.0f;

            // Dark pixels (below threshold) become obstacles
            // This means SVG strokes/fills with dark colors define walls
            if (luminance < luminanceThreshold) {
                obstacles[i] = 1;
                obstacleCount++;
            } else {
                obstacles[i] = 0;
            }
        } else {
            // Transparent pixels are empty space
            obstacles[i] = 0;
        }
    }

    float obstaclePercent = 100.0f * obstacleCount / (width * height);
    std::cout << "SVGLoader: Converted " << obstacleCount << " obstacle pixels ("
              << obstaclePercent << "%)" << std::endl;
}
