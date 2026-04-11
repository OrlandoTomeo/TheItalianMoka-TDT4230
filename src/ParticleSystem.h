#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;
    float Life;
    float Size;
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles, glm::vec3 spawnPos, bool isAdditive)
        : maxParticles(maxParticles), spawnPos(spawnPos), isAdditive(isAdditive) {
        particles.resize(maxParticles);
        for(int i=0; i<maxParticles; ++i) resetParticle(particles[i]);
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Riserviamo memoria: 8 float per particella (3 Pos, 4 Color, 1 Size)
        glBufferData(GL_ARRAY_BUFFER, maxParticles * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0); 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); 
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); 
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    }

    void Update(float dt) {
        particleData.clear();
        for(auto& p : particles) {
            p.Life -= dt;
            if(p.Life > 0.0f) {
                p.Position += p.Velocity * dt;
                p.Color.a = (p.Life / 2.0f); // Dissolvenza
                
                particleData.push_back(p.Position.x); particleData.push_back(p.Position.y); particleData.push_back(p.Position.z);
                particleData.push_back(p.Color.r); particleData.push_back(p.Color.g); particleData.push_back(p.Color.b); particleData.push_back(p.Color.a);
                particleData.push_back(p.Size);
            } else {
                resetParticle(p);
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particleData.size() * sizeof(float), particleData.data());
    }

    void Draw(Shader& shader) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        // Blending additivo per il fuoco, normale per il fumo
        if(isAdditive) glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
        else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particleData.size() / 8);
        glDisable(GL_BLEND);
    }

private:
    std::vector<Particle> particles;
    std::vector<float> particleData;
    int maxParticles;
    glm::vec3 spawnPos;
    bool isAdditive;
    unsigned int VAO, VBO;

    float randomFloat(float min, float max) {
        return min + (max - min) * (rand() / (float)RAND_MAX);
    }

    void resetParticle(Particle& p) {
        if(isAdditive) { // CONFIGURAZIONE FUOCO (FORNELLO AD ANELLO)
            // Crea un anello invece di un punto centrale
            float angle = randomFloat(0.0f, 3.14159f * 2.0f); // Angolo casuale a 360 gradi
            float ringRadius = randomFloat(0.55f, 0.75f);     // Raggio del fornello (allargalo se serve)
            
            p.Position = spawnPos + glm::vec3(cos(angle) * ringRadius, randomFloat(-0.05f, 0.05f), sin(angle) * ringRadius);
            
            // Il fuoco va verso l'alto e leggermente verso il centro (per avvolgere la moka)
            p.Velocity = glm::vec3(cos(angle + 3.14f)*0.1f, randomFloat(0.4f, 0.8f), sin(angle + 3.14f)*0.1f);
            
            // Colori caldi e vibranti (Giallo/Arancio base)
            p.Color = glm::vec4(1.0f, randomFloat(0.3f, 0.7f), 0.1f, 1.0f);
            p.Size = randomFloat(0.15f, 0.35f);
            p.Life = randomFloat(0.4f, 1.0f); // Vita breve, fiamma nervosa
            
        } else { // CONFIGURAZIONE VAPORE (PUNTO SINGOLO SUL BECCUCCIO)
            p.Position = spawnPos + glm::vec3(randomFloat(-0.05f, 0.05f), 0.0f, randomFloat(-0.05f, 0.05f));
            p.Velocity = glm::vec3(randomFloat(-0.1f, 0.1f), randomFloat(0.8f, 1.5f), randomFloat(-0.1f, 0.1f));
            p.Color = glm::vec4(0.9f, 0.9f, 0.9f, 0.3f); // Vapore più trasparente
            p.Size = randomFloat(0.3f, 0.8f);
            p.Life = randomFloat(1.5f, 3.0f);
        }
    }
    void setSpawnPosition(glm::vec3 newPos) {
        spawnPos = newPos;
    }
};
#endif