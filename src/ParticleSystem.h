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
    // FIX: Sostituito bool isAdditive con int particleType
    // 0 = Fuoco
    // 1 = Vapore
    // 2 = Caffè
    ParticleSystem(int maxParticles, glm::vec3 spawnPos, int particleType)
        : maxParticles(maxParticles), spawnPos(spawnPos), pType(particleType) {
        particles.resize(maxParticles);
        for(int i = 0; i < maxParticles; ++i) resetParticle(particles[i]);
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0); 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); 
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); 
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    }

    void Update(float dt, bool emitNew = true) {
        particleData.clear();
        
        glm::vec3 outwardDir = glm::vec3(0.0f);
        if(pType == 1) { // Solo vapore
            outwardDir = glm::normalize(glm::vec3(spawnPos.x, 0.0f, spawnPos.z));
        }

        for(auto& p : particles) {
            p.Life -= dt;
            if(p.Life > 0.0f) {
                
                // --- VAPORE: Fake collision col coperchio ---
                if (pType == 1) { 
                    float lidHeight = spawnPos.y + 1.1f; 
                    float currentLidHeight = lidHeight + (p.Position.x * 0.2f) + (p.Position.z * 0.1f);
                    
                    if (p.Position.y > currentLidHeight) {
                        p.Velocity.y *= 0.75f; 
                        p.Velocity.x = -0.9f; 
                        p.Velocity.y =  0.45f; 
                        p.Velocity.z = -0.3f; 
                        
                        p.Velocity.x += randomFloat(-0.5f, 0.5f);
                        p.Velocity.y += randomFloat(-0.1f, 0.1f);
                        p.Velocity.z += randomFloat(-0.5f, 0.5f);
                    }
                }
                
                // --- CAFFÈ: Fermarlo quando tocca la moka ---
                if (pType == 2) {
                    // Il caffè cade (Y diminuisce).
                    // Supponiamo che il "fondo" della moka sia a Y = -0.5f (modifica questo valore se sbava sotto)
                    if (p.Position.y < -0.5f) { 
                        p.Life = -1.0f; // Uccidiamo la particella appena tocca il fondo
                    }
                }

                // Applica la velocità alla posizione
                p.Position += p.Velocity * dt;
                
                // Dissolvenza alfa in base alla vita
                p.Color.a = (p.Life / 1.0f); 
                
                // Salva i dati
                particleData.push_back(p.Position.x); particleData.push_back(p.Position.y); particleData.push_back(p.Position.z);
                particleData.push_back(p.Color.r); particleData.push_back(p.Color.g); particleData.push_back(p.Color.b); particleData.push_back(p.Color.a);
                particleData.push_back(p.Size);
            } else {
                if (emitNew) {
                    resetParticle(p);
                }
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particleData.size() * sizeof(float), particleData.data());
    }

    void Draw(Shader& shader) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        
        // Se è fuoco (0) fa "Glow", altrimenti fa standard blending (vapore o caffè)
        if(pType == 0) glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
        else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particleData.size() / 8);
        glDisable(GL_BLEND);
    }

    void setSpawnPosition(glm::vec3 newPos) {
        spawnPos = newPos;
    }

private:
    std::vector<Particle> particles;
    std::vector<float> particleData;
    int maxParticles;
    glm::vec3 spawnPos;
    int pType; // 0=Fuoco, 1=Vapore, 2=Caffè
    unsigned int VAO, VBO;

    float randomFloat(float min, float max) {
        return min + (max - min) * (rand() / (float)RAND_MAX);
    }

    void resetParticle(Particle& p) {
        if(pType == 0) { 
            // 0. FIRE CONFIGURATION
            float angle = randomFloat(0.0f, 3.14159f * 2.0f); 
            float ringRadius = 0.65f; 
            
            p.Position = spawnPos + glm::vec3(cos(angle) * ringRadius, randomFloat(-0.02f, 0.02f), sin(angle) * ringRadius);
            p.Velocity = glm::vec3(0.0f, randomFloat(0.4f, 0.8f), 0.0f); 
            p.Color = glm::vec4(1.0f, randomFloat(0.3f, 0.7f), 0.1f, 1.0f);
            p.Size = randomFloat(0.15f, 0.35f);
            p.Life = randomFloat(0.4f, 0.8f); 
            
        } else if (pType == 1) { 
            // 1. STEAM CONFIGURATION
            p.Position = spawnPos + glm::vec3(randomFloat(-0.05f, 0.05f), 0.0f, randomFloat(-0.05f, 0.05f));
            p.Velocity = glm::vec3(randomFloat(-0.1f, 0.1f), randomFloat(0.8f, 1.5f), randomFloat(-0.1f, 0.1f));
            p.Color = glm::vec4(0.9f, 0.9f, 0.9f, 0.3f); 
            p.Size = randomFloat(0.3f, 0.8f);
            p.Life = randomFloat(1.5f, 3.0f);
            
        } else if (pType == 2) {
            // 2. COFFEE CONFIGURATION (NUOVO!)
            // Il caffè nasce dal beccuccio (concentrato) e va VERSO IL BASSO (Y negativa)
            p.Position = spawnPos + glm::vec3(randomFloat(-0.02f, 0.02f), 0.0f, randomFloat(-0.02f, 0.02f));
            
            // Va lentamente in giù sull'asse Y. Leggera spinta a sinistra (su X) per scivolare sulla moka
            p.Velocity = glm::vec3(randomFloat(-0.01f, -0.05f), randomFloat(-0.4f, -0.2f), randomFloat(-0.02f, 0.02f));
            
            // Colore marrone scurissimo (caffè)
            p.Color = glm::vec4(0.15f, 0.05f, 0.0f, 1.0f); 
            
            // Dimensioni piccole! (È una goccia, non una nuvola di fumo)
            p.Size = randomFloat(0.15f, 0.20f);
            
            // Vita breve (muore in fretta)
            p.Life = randomFloat(1.0f, 1.5f);
        }
    }
};
#endif