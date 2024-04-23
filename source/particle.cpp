#include <glad/glad.h>

#include "particle.h"

#define OPTIMIZE


ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
    : shader(shader), texture(texture), amount(amount) {
    this->init();
}

ParticleGenerator::~ParticleGenerator() {
    delete (instance_data);
}


void ParticleGenerator::init() {
    unsigned int VBO;

    float particle_quad[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glGenBuffers(1, &instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * amount, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    for (unsigned int i = 0; i < this->amount; i++)
        this->particles.emplace_back(Particle());
    
    instance_data = new float[this->amount * 6];
}


void ParticleGenerator::Update(float dt, GameObject &object, unsigned int newParticles, glm::vec2 offset) {
    // add new particles 
    for (unsigned int i = 0; i < newParticles; ++i) {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], object, offset);
    }
    // update all particles
    for (unsigned int i = 0; i < this->amount; ++i) {
        Particle &p = this->particles[i];
        p.Life -= dt;   // reduce life
        if (p.Life > 0.0f) {
            p.Position -= p.Velocity * dt; 
            p.Color.a -= dt * 2.5f;
        }
    }
}


unsigned int lastUsedParticle = 0;

unsigned int ParticleGenerator::firstUnusedParticle() {
    // first search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedParticle; i < this->amount; ++i){
        if (this->particles[i].Life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i){
        if (this->particles[i].Life <= 0.0f){
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one
    lastUsedParticle = 0;
    return 0;
}


void ParticleGenerator::respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle.Position = object.Position + random + offset;
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 0.8f;
    particle.Velocity = object.Velocity * 0.1f;
}


void ParticleGenerator::Draw() {
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    this->shader.Use();

//     for (Particle &particle : this->particles) {
//         if (particle.Life > 0.0f) {
//             this->shader.SetVector2f("offset", particle.Position);
//             this->shader.SetVector4f("color", particle.Color);
//             this->texture.Bind();
//             glBindVertexArray(this->VAO);

//             glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//             glBindVertexArray(0);
//         }
//     }

    this->texture.Bind();
    glBindVertexArray(this->VAO);

    unsigned int cnt = 0;
    for (Particle &particle : this->particles) {
        if (particle.Life > 0.0f) {
            instance_data[cnt++] = particle.Position.x;
            instance_data[cnt++] = particle.Position.y;
            instance_data[cnt++] = particle.Color.r;
            instance_data[cnt++] = particle.Color.g;
            instance_data[cnt++] = particle.Color.b;
            instance_data[cnt++] = particle.Color.a;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, this->instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * cnt, instance_data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, cnt / 6);
    glBindVertexArray(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
