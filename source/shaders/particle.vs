#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

// After optimize ...
layout (location = 1) in vec2 offset;
layout (location = 2) in vec4 color;

out vec4 ParticleColor;
out vec2 TexCoords;

uniform mat4 projection;

// After optimize ...
// uniform vec2 offset;
// uniform vec4 color;


void main() {
    float scale = 10.0f;
    TexCoords = vertex.zw;
    ParticleColor = color;
    gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
}
