#ifndef PARTICLE_H
#define PARTICLE_H

#include <vector>
#include <glm/glm.hpp>

#include "shader.h"
#include "texture.h"
#include "object.h"


class Particle {
public:
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float Life;

public:
    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};


class ParticleGenerator {
public:
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);
    ~ParticleGenerator();

    void Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
    void Draw();

private:
    std::vector<Particle> particles;
    unsigned int amount;

    Shader shader;
    Texture2D texture;
    unsigned int VAO;

    float* instance_data;
    unsigned int instance_vbo;

    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};

#endif
