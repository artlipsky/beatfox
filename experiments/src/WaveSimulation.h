#pragma once

#include <vector>
#include <cmath>

class WaveSimulation {
public:
    WaveSimulation(int width, int height);
    ~WaveSimulation();

    void update(float dt);
    void addDisturbance(int x, int y, float amplitude);
    void addPressureSource(int x, int y, float pressure);
    void clear();

    // Getters
    const float* getData() const { return pressure.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Parameters
    void setWaveSpeed(float speed) { soundSpeed = speed; }
    void setDamping(float damp) { damping = damp; }
    float getWaveSpeed() const { return soundSpeed; }
    float getDamping() const { return damping; }

    // Physical dimensions
    float getPhysicalWidth() const { return width * dx; }  // meters
    float getPhysicalHeight() const { return height * dx; } // meters

    // Obstacles
    void addObstacle(int x, int y, int radius);
    void removeObstacle(int x, int y, int radius);
    void clearObstacles();
    bool isObstacle(int x, int y) const;
    const uint8_t* getObstacles() const { return obstacles.data(); }

private:
    int width;              // Grid width (pixels)
    int height;             // Grid height (pixels)
    float soundSpeed;       // Speed of sound in air (m/s)
    float damping;          // Air absorption coefficient (dimensionless)
    float wallReflection;   // Wall reflection coefficient (0-1, energy loss at walls)
    float dx;               // Spatial step: 1 pixel = 1 cm = 0.01 m

    // Acoustic pressure field (deviation from ambient pressure)
    std::vector<float> pressure;      // Current pressure field (Pa)
    std::vector<float> pressurePrev;  // Previous pressure field (Pa)
    std::vector<float> pressureNext;  // Next pressure field (Pa)

    // Obstacle field (solid objects that block sound)
    std::vector<uint8_t> obstacles;   // 1 if cell contains an obstacle, 0 otherwise

    void updateStep(float dt);  // Single time step

    int index(int x, int y) const {
        return y * width + x;
    }
};
