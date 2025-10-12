#version 330 core

in float waveHeight;
in vec2 fragPos;

out vec4 FragColor;

// Color mapping based on wave height
vec3 heightToColor(float h) {
    // Normalize height to [0, 1] range (assuming heights are roughly -1 to 1)
    float t = (h + 1.0) * 0.5;

    // Create a nice color gradient
    vec3 color;
    if (t < 0.25) {
        // Blue to Cyan
        float s = t / 0.25;
        color = mix(vec3(0.0, 0.0, 0.5), vec3(0.0, 0.5, 1.0), s);
    } else if (t < 0.5) {
        // Cyan to Green
        float s = (t - 0.25) / 0.25;
        color = mix(vec3(0.0, 0.5, 1.0), vec3(0.0, 1.0, 0.5), s);
    } else if (t < 0.75) {
        // Green to Yellow
        float s = (t - 0.5) / 0.25;
        color = mix(vec3(0.0, 1.0, 0.5), vec3(1.0, 1.0, 0.0), s);
    } else {
        // Yellow to Red
        float s = (t - 0.75) / 0.25;
        color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.2, 0.0), s);
    }

    return color;
}

void main() {
    vec3 color = heightToColor(waveHeight);

    // Add some ambient lighting based on wave height
    float brightness = 0.7 + 0.3 * abs(waveHeight);
    color *= brightness;

    FragColor = vec4(color, 1.0);
}
