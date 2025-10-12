#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in float height;

out float waveHeight;
out vec2 fragPos;

uniform mat4 projection;

void main() {
    fragPos = position;
    waveHeight = height;
    gl_Position = projection * vec4(position.x, position.y, 0.0, 1.0);
}
