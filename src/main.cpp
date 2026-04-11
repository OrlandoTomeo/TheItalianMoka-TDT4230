#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>

#include "stb_image.h"
#include "Shader.h"
#include "Model.h"
#include "ParticleSystem.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Load Cubemap
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

// --- FUNZIONE EXTRA: Genera un cilindro in C++ per la base del fornello ---
void buildCylinder(float radius, float height, int sectors, unsigned int& VAO, unsigned int& VBO, int& vertexCount) {
    std::vector<float> vertices;
    float sectorStep = 2 * 3.1415926f / sectors;
    
    for(int i = 0; i < sectors; ++i) {
        float a1 = i * sectorStep;
        float a2 = (i + 1) * sectorStep;
        
        float x1 = cos(a1) * radius, z1 = sin(a1) * radius;
        float x2 = cos(a2) * radius, z2 = sin(a2) * radius;
        
        float nx1 = cos(a1), nz1 = sin(a1);
        float nx2 = cos(a2), nz2 = sin(a2);
        
        // Lati del cilindro
        vertices.insert(vertices.end(), {x1, height/2, z1, nx1, 0.0f, nz1, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x1, -height/2, z1, nx1, 0.0f, nz1, 0.0f, 1.0f});
        vertices.insert(vertices.end(), {x2, -height/2, z2, nx2, 0.0f, nz2, 1.0f, 1.0f});
        
        vertices.insert(vertices.end(), {x1, height/2, z1, nx1, 0.0f, nz1, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x2, -height/2, z2, nx2, 0.0f, nz2, 1.0f, 1.0f});
        vertices.insert(vertices.end(), {x2, height/2, z2, nx2, 0.0f, nz2, 1.0f, 0.0f});
        
        // Tappo superiore
        vertices.insert(vertices.end(), {0.0f, height/2, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f});
        vertices.insert(vertices.end(), {x1, height/2, z1, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x2, height/2, z2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f});
    }
    
    vertexCount = vertices.size() / 8;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Italian Moka Ritual", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    Shader mokaShader("shaders/moka.vert", "shaders/moka.frag");
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag"); 
    Shader envShader("shaders/env.vert", "shaders/env.frag");
    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");

    std::vector<std::string> faces = { "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);

    // --- SETUP AMBIENTE: PAVIMENTO ---
    float floorVertices[] = {
        // Posizioni (Piano molto grande)  // Normali (verso l'alto)  // UV
         20.0f, 0.0f,  20.0f,              0.0f, 1.0f, 0.0f,          10.0f, 0.0f,
        -20.0f, 0.0f,  20.0f,              0.0f, 1.0f, 0.0f,          0.0f,  0.0f,
        -20.0f, 0.0f, -20.0f,              0.0f, 1.0f, 0.0f,          0.0f,  10.0f,

         20.0f, 0.0f,  20.0f,              0.0f, 1.0f, 0.0f,          10.0f, 0.0f,
        -20.0f, 0.0f, -20.0f,              0.0f, 1.0f, 0.0f,          0.0f,  10.0f,
         20.0f, 0.0f, -20.0f,              0.0f, 1.0f, 0.0f,          10.0f, 10.0f
    };
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // --- SETUP AMBIENTE: FORNELLO (CILINDRO) ---
    unsigned int cylVAO, cylVBO;
    int cylVertexCount;
    // Raggio 0.75 (più largo del fuoco), Altezza 0.05, 32 lati per farlo tondo
    buildCylinder(0.75f, 0.05f, 32, cylVAO, cylVBO, cylVertexCount);

    // Variabili fisse con i tuoi dati perfetti
    glm::vec3 currentFirePos = glm::vec3(-0.262055f, -1.48022f, -0.674087f);
    glm::vec3 currentSteamPos = glm::vec3(-0.167861f, 0.266838f, -0.473491f);
    
    ParticleSystem fireSystem(150, currentFirePos, true); 
    ParticleSystem steamSystem(100, currentSteamPos, false); 

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);

        // Sfondo da Studio scuro ed elegante
        glClearColor(0.08f, 0.06f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- CAMERA FISSA ---
        float camY = 2.0f + sin(currentFrame * 0.3f) * 0.5f; 
        glm::vec3 cameraPos = glm::vec3(-3.2f, 1.8f, 3.8f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // --- LUCE FUOCO ---
        float flickerIntensity = 0.8f + (sin(currentFrame * 15.0f) + cos(currentFrame * 21.0f)) * 0.1f;
        glm::vec3 fireColor = glm::vec3(5.0f, 2.0f, 0.2f) * flickerIntensity;

        // =========================================================
        // 1. RENDER MOKA (SHADER ORIGINALE)
        // =========================================================
        mokaShader.use();
        mokaShader.setVec3("cameraPos", cameraPos);
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);
        mokaShader.setVec3("firePos", currentFirePos);
        mokaShader.setVec3("fireColor", fireColor);
        mokaShader.setFloat("fireConstant", 1.0f);
        mokaShader.setFloat("fireLinear", 0.09f);
        mokaShader.setFloat("fireQuadratic", 0.032f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        mokaShader.setInt("skybox", 0);

        glm::mat4 mokaModel = glm::mat4(1.0f);
        mokaModel = glm::translate(mokaModel, glm::vec3(0.0f, -0.8f, 0.0f));
        mokaModel = glm::rotate(mokaModel, glm::radians(19.4863f), glm::vec3(1.0f, 0.0f, 0.0f));
        mokaModel = glm::rotate(mokaModel, glm::radians(196.127f), glm::vec3(0.0f, 1.0f, 0.0f));
        mokaModel = glm::scale(mokaModel, glm::vec3(1.2f));
        mokaShader.setMat4("model", mokaModel);
        mokaBialetti.Draw(mokaShader);

        // =========================================================
        // 2. RENDER AMBIENTE (TUTTI GLI OGGETTI CON ENV SHADER)
        // =========================================================
        envShader.use(); // <--- QUESTO E' QUELLO CHE MANCAVA!
        envShader.setVec3("cameraPos", cameraPos);
        envShader.setMat4("view", view);
        envShader.setMat4("projection", projection);
        envShader.setVec3("firePos", currentFirePos);
        envShader.setVec3("fireColor", fireColor);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        envShader.setInt("skybox", 0);

        // --- 2A. FORNELLO ---
        envShader.setVec3("albedo", glm::vec3(0.03f, 0.03f, 0.03f)); 
        envShader.setFloat("metallic", 0.7f); 
        envShader.setFloat("roughness", 0.85f); 
        envShader.setInt("isCastIron", 1);
        glm::mat4 cylModel = glm::mat4(1.0f);
        cylModel = glm::translate(cylModel, glm::vec3(currentFirePos.x, -1.52f, currentFirePos.z));
        envShader.setMat4("model", cylModel);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- 2B. PAVIMENTO ---
        envShader.setVec3("albedo", glm::vec3(0.12f, 0.09f, 0.07f)); 
        envShader.setFloat("metallic", 0.0f); 
        envShader.setFloat("roughness", 0.90f); 
        envShader.setInt("isCastIron", 0);
        glm::mat4 floorModel = glm::mat4(1.0f);
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -1.55f, 0.0f));
        envShader.setMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // =========================================================
        // 2C. MURI DELLA CUCINA (Opachi al 100%)
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.85f, 0.82f, 0.75f)); 
        envShader.setFloat("metallic", 0.0f);  
        
        // FIX RIFLESSO: Portiamo la roughness quasi al massimo (0.95). 
        // Ora il muro è puro colore/intonaco opaco e non riflette la luce come vetro.
        envShader.setFloat("roughness", 0.95f);  
        envShader.setInt("isCastIron", 0); 
        
        // MURO POSTERIORE
        glm::mat4 wallModel = glm::mat4(1.0f);
        wallModel = glm::translate(wallModel, glm::vec3(0.0f, -1.0f, -4.0f)); 
        wallModel = glm::rotate(wallModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        envShader.setMat4("model", wallModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // MURO DI DESTRA
        glm::mat4 rightWall = glm::mat4(1.0f);
        rightWall = glm::translate(rightWall, glm::vec3(4.0f, -1.0f, 0.0f)); 
        rightWall = glm::rotate(rightWall, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        envShader.setMat4("model", rightWall);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // =========================================================
        // 2D. MENSOLINE DI LEGNO MODERNE
        // =========================================================
        // Usiamo un bel marrone caldo e opaco per simulare il legno opaco
        envShader.setVec3("albedo", glm::vec3(0.25f, 0.15f, 0.08f)); 
        envShader.setFloat("metallic", 0.0f);
        envShader.setFloat("roughness", 0.85f); 
        
        // Mensola 1 (In alto a sinistra)
        glm::mat4 shelf1 = glm::mat4(1.0f);
        shelf1 = glm::translate(shelf1, glm::vec3(-1.8f, 1.2f, -3.8f)); // Attaccata al muro
        // La schiacciamo in Y e la allarghiamo in X. Essendo un cilindro, verrà fuori una mensola con i bordi stondati!
        shelf1 = glm::scale(shelf1, glm::vec3(3.0f, 0.1f, 0.8f)); 
        envShader.setMat4("model", shelf1);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // Mensola 2 (Più in basso verso destra)
        glm::mat4 shelf2 = glm::mat4(1.0f);
        shelf2 = glm::translate(shelf2, glm::vec3(1.2f, 0.5f, -3.8f)); 
        shelf2 = glm::scale(shelf2, glm::vec3(2.5f, 0.1f, 0.8f)); 
        envShader.setMat4("model", shelf2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        // =========================================================
        // 3. RENDER PARTICELLE (SEMPRE PER ULTIME!)
        // =========================================================
        fireSystem.Update(deltaTime);
        steamSystem.Update(deltaTime);

        particleShader.use();
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        
        glDepthMask(GL_FALSE); 
        fireSystem.Draw(particleShader);
        steamSystem.Draw(particleShader);
        glDepthMask(GL_TRUE); 

        // CHIUSURA FRAME
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}