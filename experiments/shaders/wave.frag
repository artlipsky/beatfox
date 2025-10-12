#version 330 core

in float waveHeight;
in float isObstacle;
in vec2 fragPos;

out vec4 FragColor;

// Color mapping for acoustic pressure
// Positive pressure = red/yellow (compression)
// Zero pressure = dark gray (ambient)
// Negative pressure = blue/cyan (rarefaction)
vec3 heightToColor(float h) {
    // Scale for acoustic pressure visualization
    // Pressure is in Pascals, scale for 5 Pa (typical clap)
    float pressure = clamp(h / 5.0, -1.0, 1.0);

    vec3 color;
    if (pressure > 0.0) {
        // Positive pressure (compression): dark -> yellow -> red
        color = mix(vec3(0.15, 0.15, 0.15), vec3(1.0, 0.3, 0.0), pressure);
    } else {
        // Negative pressure (rarefaction): dark -> cyan -> blue
        color = mix(vec3(0.15, 0.15, 0.15), vec3(0.0, 0.6, 1.0), -pressure);
    }

    return color;
}

void main() {
    // Check if this is an obstacle
    if (isObstacle > 0.5) {
        // Render obstacle as solid dark object
        vec3 obstacleColor = vec3(0.05, 0.05, 0.08);  // Very dark blue-gray
        FragColor = vec4(obstacleColor, 1.0);
        return;
    }

    // Normal wave rendering
    vec3 color = heightToColor(waveHeight);

    // Add some ambient lighting based on wave height
    float brightness = 0.7 + 0.3 * abs(waveHeight);
    color *= brightness;

    FragColor = vec4(color, 1.0);
}
