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
        for(int i = 0; i < maxParticles; ++i) resetParticle(particles[i]);
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Reserve memory: 8 floats per particle (3 Pos, 4 Color, 1 Size)
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
        
        // Calcoliamo la direzione verso cui spinge il beccuccio.
        // Essendo scentrato rispetto a (0,0,0), la sua posizione X e Z ci indica 
        // esattamente la direzione "in avanti" per allontanarsi dalla moka!
        glm::vec3 outwardDir = glm::vec3(0.0f);
        if(!isAdditive) { // Se è il vapore
            outwardDir = glm::normalize(glm::vec3(spawnPos.x, 0.0f, spawnPos.z));
        }

        for(auto& p : particles) {
            p.Life -= dt;
            if(p.Life > 0.0f) {
                
                // --- INIZIO FAKE COLLISION COPERCHIO ---
                if (!isAdditive) { // Solo per il vapore
                    // Definiamo a che altezza si trova il coperchio rispetto alla nascita del vapore
                    // (0.15f significa poco sopra il beccuccio. Modificalo se sbatte troppo presto o tardi)
                    float lidHeight = spawnPos.y + 1.1f; 

                    float currentLidHeight = lidHeight + (p.Position.x * 0.2f) + (p.Position.z * 0.1f);
                    
                    if (p.Position.y > currentLidHeight) {
                        // 1. Il vapore sbatte: smorziamo la sua velocità di salita (Y)
                        p.Velocity.y *= 0.75f; 
                        
                        float slideX = -0.9f; // Spinta a sinistra/destra
                        float slideY =  0.45f; // SPINTA VERSO L'ALTO (Fa scivolare lungo il coperchio!)
                        float slideZ = -0.3f; // Spinta avanti/indietro
                        
                        p.Velocity.x = slideX;
                        p.Velocity.y = slideY;
                        p.Velocity.z = slideZ;
                        
                        // 3. Aggiungiamo un po' di turbolenza per farlo allargare ai lati (effetto nuvola)
                        p.Velocity.x += randomFloat(-0.5f, 0.5f);
                        p.Velocity.y += randomFloat(-0.1f, 0.1f);
                        p.Velocity.z += randomFloat(-0.5f, 0.5f);
                    }
                }
                // --- FINE FAKE COLLISION ---

                // Applica la velocità alla posizione
                p.Position += p.Velocity * dt;
                
                // Fai svanire dolcemente la particella nel tempo
                p.Color.a = (p.Life / 1.0f); 
                
                // Salva i dati per la scheda video
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
        
        // Additive blending for fire (glow), standard alpha blending for steam
        if(isAdditive) glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
        else glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particleData.size() / 8);
        glDisable(GL_BLEND);
    }

    // Utility function to update spawn position dynamically
    void setSpawnPosition(glm::vec3 newPos) {
        spawnPos = newPos;
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
        if(isAdditive) { 
            // FIRE CONFIGURATION (Ring Burner)
            float angle = randomFloat(0.0f, 3.14159f * 2.0f); 
            float ringRadius = 0.65f; // Radius of the gas burner ring
            
            p.Position = spawnPos + glm::vec3(cos(angle) * ringRadius, randomFloat(-0.02f, 0.02f), sin(angle) * ringRadius);
            p.Velocity = glm::vec3(0.0f, randomFloat(0.4f, 0.8f), 0.0f); // Move straight up
            
            // Warm orange/yellow colors
            p.Color = glm::vec4(1.0f, randomFloat(0.3f, 0.7f), 0.1f, 1.0f);
            p.Size = randomFloat(0.15f, 0.35f);
            p.Life = randomFloat(0.4f, 0.8f); // Short life for flickering flames
        } else { 
            // STEAM CONFIGURATION (Single Point Spout)
            p.Position = spawnPos + glm::vec3(randomFloat(-0.05f, 0.05f), 0.0f, randomFloat(-0.05f, 0.05f));
            p.Velocity = glm::vec3(randomFloat(-0.1f, 0.1f), randomFloat(0.8f, 1.5f), randomFloat(-0.1f, 0.1f));
            
            // Soft transparent white/grey
            p.Color = glm::vec4(0.9f, 0.9f, 0.9f, 0.3f); 
            p.Size = randomFloat(0.3f, 0.8f);
            p.Life = randomFloat(1.5f, 3.0f);
        }
    }
};
#endif