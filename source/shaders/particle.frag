#version 330 core

in vec4 ParticleColor;
in vec2 TexCoords;

out vec4 Color;

uniform sampler2D sprite;

void main() {
    Color = texture(sprite, TexCoords) * ParticleColor;
}
