#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cmath>
//#include <string>
#include "stb_image.h"
#include "Shader.h"
#include "Model.h"
#include "ParticleSystem.h"
//#include "audioManager.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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
        
        vertices.insert(vertices.end(), {x1, height/2, z1, nx1, 0.0f, nz1, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x1, -height/2, z1, nx1, 0.0f, nz1, 0.0f, 1.0f});
        vertices.insert(vertices.end(), {x2, -height/2, z2, nx2, 0.0f, nz2, 1.0f, 1.0f});
        
        vertices.insert(vertices.end(), {x1, height/2, z1, nx1, 0.0f, nz1, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x2, -height/2, z2, nx2, 0.0f, nz2, 1.0f, 1.0f});
        vertices.insert(vertices.end(), {x2, height/2, z2, nx2, 0.0f, nz2, 1.0f, 0.0f});
        
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

    /*
    //audiomanager
    SpaceEngine::AudioManager audioManager;
    ...
    */
    std::vector<std::string> faces = { "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);

    float floorVertices[] = {
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

    unsigned int cylVAO, cylVBO;
    int cylVertexCount;
    buildCylinder(0.75f, 0.05f, 32, cylVAO, cylVBO, cylVertexCount);

    glm::vec3 currentFirePos = glm::vec3(-0.262055f, -1.48022f, -0.674087f);
    glm::vec3 currentSteamPos = glm::vec3(-0.167861f, 0.266838f, -0.473491f);
    glm::vec3 coffeeSpoutPos = glm::vec3(-0.167861f, 1.05f, -0.473491f);

    ParticleSystem fireSystem(150, currentFirePos, 0); 
    ParticleSystem steamSystem(50, currentSteamPos, 1); 
    ParticleSystem coffeeSystem(15, coffeeSpoutPos, 2);
    // === SHADOW MAPPING SETUP ===
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Evita che le ombre si ripetano fuori dai bordi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader shadowShader("shaders/shadow.vert", "shaders/shadow.frag");
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);

        // =========================================================
        // PASS 1: RENDER DELLA SHADOW MAP (LA FOTO DALLA LUCE)
        // =========================================================
        glm::vec3 lightPos = glm::vec3(-8.0f, 8.0f, 4.0f); 
        
        // Allarghiamo la telecamera da 10 a 15 per catturare le ombre lunghe
        glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 1.0f, 30.0f);
        
        // Siccome la luce non guarda più "a piombo", rimettiamo l'Up Vector standard (0,1,0)
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT); 
        
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // --- DISEGNIAMO LA MOKA NELL'OMBRA ---
        glm::mat4 mokaShadowModel = glm::mat4(1.0f);
        mokaShadowModel = glm::translate(mokaShadowModel, glm::vec3(0.0f, -0.8f, 0.0f));
        mokaShadowModel = glm::rotate(mokaShadowModel, glm::radians(19.4863f), glm::vec3(1.0f, 0.0f, 0.0f));
        mokaShadowModel = glm::rotate(mokaShadowModel, glm::radians(196.127f), glm::vec3(0.0f, 1.0f, 0.0f));
        mokaShadowModel = glm::scale(mokaShadowModel, glm::vec3(1.2f));
        shadowShader.setMat4("model", mokaShadowModel);
        mokaBialetti.Draw(shadowShader);

        // --- DISEGNIAMO MENSOLE E TAZZINE NELL'OMBRA ---
        glBindVertexArray(cylVAO); // Tutto il resto usa i cilindri
        
        // Mensole
        glm::mat4 sh1 = glm::mat4(1.0f); sh1 = glm::translate(sh1, glm::vec3(-0.6f, 1.6f, -3.8f)); sh1 = glm::scale(sh1, glm::vec3(2.5f, 1.12f, 0.6f)); 
        shadowShader.setMat4("model", sh1); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        
        glm::mat4 sh2 = glm::mat4(1.0f); sh2 = glm::translate(sh2, glm::vec3(-0.6f, 0.9f, -3.8f)); sh2 = glm::scale(sh2, glm::vec3(2.5f, 1.12f, 0.6f)); 
        shadowShader.setMat4("model", sh2); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // MENSOLA DESTRA FIXATA: X a 4.3f
        glm::mat4 shR = glm::mat4(1.0f); shR = glm::translate(shR, glm::vec3(4.3f, 1.2f, 1.0f)); shR = glm::scale(shR, glm::vec3(1.0f, 1.12f, 2.5f)); 
        shadowShader.setMat4("model", shR); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // Tazzine (Ne bastano le forme base per fare un'ottima ombra)
        glm::mat4 c1 = glm::mat4(1.0f); c1 = glm::translate(c1, glm::vec3(-0.2f, 1.05f, -3.75f)); c1 = glm::scale(c1, glm::vec3(0.20f, 6.5f, 0.20f)); shadowShader.setMat4("model", c1); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        glm::mat4 c2 = glm::mat4(1.0f); c2 = glm::translate(c2, glm::vec3(0.4f, 1.05f, -3.75f)); c2 = glm::scale(c2, glm::vec3(0.20f, 6.5f, 0.20f)); shadowShader.setMat4("model", c2); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        glm::mat4 c3 = glm::mat4(1.0f); c3 = glm::translate(c3, glm::vec3(-1.5f, 1.75f, -3.75f)); c3 = glm::scale(c3, glm::vec3(0.20f, 6.5f, 0.20f)); shadowShader.setMat4("model", c3); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        
        // TAZZINE DESTRA FIXATE: X a 4.3f
        glm::mat4 c4 = glm::mat4(1.0f); c4 = glm::translate(c4, glm::vec3(4.3f, 1.35f, 0.4f)); c4 = glm::scale(c4, glm::vec3(0.20f, 6.5f, 0.20f)); shadowShader.setMat4("model", c4); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        glm::mat4 c5 = glm::mat4(1.0f); c5 = glm::translate(c5, glm::vec3(4.3f, 1.35f, 1.0f)); c5 = glm::scale(c5, glm::vec3(0.20f, 6.5f, 0.20f)); shadowShader.setMat4("model", c5); glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // =========================================================
        // PASS 2: RENDER NORMALE DELLA SCENA
        // =========================================================
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        // Da qui in poi è esattamente il tuo codice di prima
        glClearColor(0.40f, 0.43f, 0.47f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float camY = 2.0f + sin(currentFrame * 0.3f) * 0.5f; 
        glm::vec3 cameraPos = glm::vec3(-3.2f, 1.8f, 3.8f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        float flickerIntensity = 0.8f + (sin(currentFrame * 15.0f) + cos(currentFrame * 21.0f)) * 0.1f;
        glm::vec3 fireColor = glm::vec3(2.0f, 1.0f, 0.2f) * flickerIntensity;

        // =========================================================
        // 1. RENDER MOKA
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
        // 2. RENDER AMBIENTE
        // =========================================================
        envShader.use();
        envShader.setVec3("cameraPos", cameraPos);
        envShader.setMat4("view", view);
        envShader.setMat4("projection", projection);
        envShader.setVec3("firePos", currentFirePos);
        envShader.setVec3("fireColor", fireColor);
        
        envShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        envShader.setInt("skybox", 0);

        glActiveTexture(GL_TEXTURE1);                            
        glBindTexture(GL_TEXTURE_2D, depthMap);                  
        envShader.setInt("shadowMap", 1);

        // ---- 2A. MATERIALE GHISA ----
        envShader.setVec3("albedo", glm::vec3(0.01f, 0.01f, 0.01f)); 
        envShader.setFloat("metallic", 0.9f);  
        envShader.setFloat("roughness", 0.85f); 
        envShader.setInt("isCastIron", 1);
        
        // --- FORNELLO 1 (PRINCIPALE, ACCESO) ---
        glm::mat4 cylModel = glm::mat4(1.0f);
        cylModel = glm::translate(cylModel, glm::vec3(currentFirePos.x, -1.54f, currentFirePos.z));
        cylModel = glm::scale(cylModel, glm::vec3(1.1f, 1.6f, 1.1f)); 
        envShader.setMat4("model", cylModel);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 ringModel = glm::mat4(1.0f);
        ringModel = glm::translate(ringModel, glm::vec3(currentFirePos.x, -1.45f, currentFirePos.z));
        ringModel = glm::scale(ringModel, glm::vec3(0.9f, 0.08f, 0.9f));
        envShader.setMat4("model", ringModel);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 grid1 = glm::mat4(1.0f);
        grid1 = glm::translate(grid1, glm::vec3(currentFirePos.x, -1.41f, currentFirePos.z));
        grid1 = glm::scale(grid1, glm::vec3(1.2f, 0.02f, 0.1f));
        envShader.setMat4("model", grid1);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 grid2 = glm::mat4(1.0f);
        grid2 = glm::translate(grid2, glm::vec3(currentFirePos.x, -1.41f, currentFirePos.z));
        grid2 = glm::rotate(grid2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        grid2 = glm::scale(grid2, glm::vec3(1.2f, 0.02f, 0.1f));
        envShader.setMat4("model", grid2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // =========================================================
        // [MODULI DA TESTARE: FORNELLI EXTRA]
        // Modifica stepX per allontanarli a destra/sinistra.
        // Modifica stepZ per allontanarli in avanti/indietro.
        // =========================================================
        float stepX = 3.0f; 
        float stepZ = 3.0f; 

        std::vector<glm::vec3> fuochiExtra = {
            glm::vec3(currentFirePos.x + stepX, 0.0f, currentFirePos.z),         
            glm::vec3(currentFirePos.x,         0.0f, currentFirePos.z + stepZ),  
            glm::vec3(currentFirePos.x + stepX, 0.0f, currentFirePos.z + stepZ)   
        };

        for(glm::vec3 pos : fuochiExtra) {
            float fX = pos.x;
            float fZ = pos.z;

            glm::mat4 mCyl = glm::mat4(1.0f);
            mCyl = glm::translate(mCyl, glm::vec3(fX, -1.54f, fZ));
            mCyl = glm::scale(mCyl, glm::vec3(1.1f, 1.6f, 1.1f)); 
            envShader.setMat4("model", mCyl);
            glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

            glm::mat4 mRing = glm::mat4(1.0f);
            mRing = glm::translate(mRing, glm::vec3(fX, -1.45f, fZ));
            mRing = glm::scale(mRing, glm::vec3(0.9f, 0.08f, 0.9f));
            envShader.setMat4("model", mRing);
            glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

            glm::mat4 mG1 = glm::mat4(1.0f);
            mG1 = glm::translate(mG1, glm::vec3(fX, -1.41f, fZ));
            mG1 = glm::scale(mG1, glm::vec3(1.2f, 0.02f, 0.1f));
            envShader.setMat4("model", mG1);
            glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

            glm::mat4 mG2 = glm::mat4(1.0f);
            mG2 = glm::translate(mG2, glm::vec3(fX, -1.41f, fZ));
            mG2 = glm::rotate(mG2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            mG2 = glm::scale(mG2, glm::vec3(1.2f, 0.02f, 0.1f));
            envShader.setMat4("model", mG2);
            glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);
        }

        // =========================================================
        // [MODULI DA TESTARE: PAVIMENTO E CORNICIONE]
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.11f, 0.08f, 0.06f)); 
        envShader.setFloat("metallic", 0.0f); 
        envShader.setFloat("roughness", 1.0f); 
        envShader.setInt("isCastIron", 0);
        
        // --- PAVIMENTO ---
        glm::mat4 floorModel = glm::mat4(1.0f);
        floorModel = glm::translate(floorModel, glm::vec3(7.5f, -1.55f, 0.0f));
        floorModel = glm::scale(floorModel, glm::vec3(0.5f, 1.0f, 0.5f)); 
        envShader.setMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- CORNICIONE ---
        envShader.setVec3("albedo", glm::vec3(0.07f, 0.05f, 0.04f)); 
        envShader.setFloat("roughness", 0.85f);
        
        glm::mat4 edgeModel = glm::mat4(1.0f);
        edgeModel = glm::translate(edgeModel, glm::vec3(-2.5f, -1.55f, 0.0f)); 
        edgeModel = glm::rotate(edgeModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // LUNGHEZZA CORNICIONE: Nello scale qui sotto, il numero "80.0f" è la lunghezza lungo l'asse Z. 
        // L'ho aumentato tantissimo per coprire tutto, se sborda troppo dietro abbassalo a 50.0f o simili.
        edgeModel = glm::scale(edgeModel, glm::vec3(0.08f, 160.0f, 0.08f)); 
        envShader.setMat4("model", edgeModel);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // =========================================================
        // 2C. MURI DELLA CUCINA 
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.87f, 0.84f, 0.77f)); 
        
        // MURO POSTERIORE (Tutto opaco)
        envShader.setFloat("metallic", 0.0f);  
        envShader.setFloat("roughness", 1.0f); 
        envShader.setInt("isCastIron", 0); 
        
        glm::mat4 wallModel = glm::mat4(1.0f);
        wallModel = glm::translate(wallModel, glm::vec3(0.0f, -1.0f, -4.0f)); 
        wallModel = glm::rotate(wallModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        envShader.setMat4("model", wallModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // MURO DESTRA (Con FIX Riflesso minimo)
        // Ho impostato il metallo bassissimo (0.02) e roughness (0.85) in modo che 
        // catturi appena appena una lieve traccia della stanza, senza abbagliare.
        envShader.setFloat("metallic", 0.0f);  
        envShader.setFloat("roughness", 1.0f); 
        
        glm::mat4 rightWall = glm::mat4(1.0f);
        rightWall = glm::translate(rightWall, glm::vec3(5.0f, -1.0f, 0.0f)); 
        rightWall = glm::rotate(rightWall, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        envShader.setMat4("model", rightWall);
        glDrawArrays(GL_TRIANGLES, 0, 6);

       // =========================================================
        // [MODULI DA TESTARE: MENSOLE E TAZZINE]
        // =========================================================
        // =========================================================
        // Tubi metallici (Muro Destro)
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.02f, 0.02f, 0.02f)); 
        envShader.setFloat("metallic", 0.9f);
        envShader.setFloat("roughness", 0.6f); 
        
        // TUBO DESTRA 1 (Più verso il fondo)
        glm::mat4 pipeRight1 = glm::mat4(1.0f);
        // TUBI DESTRA FIXATI: X a 4.3f
        pipeRight1 = glm::translate(pipeRight1, glm::vec3(4.3f, 1.5f, 0.2f)); 
        // Scala Y per l'altezza del tubo
        pipeRight1 = glm::scale(pipeRight1, glm::vec3(0.04f, 1.2f, 0.04f)); 
        envShader.setMat4("model", pipeRight1);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // TUBO DESTRA 2 (Più verso la telecamera)
        glm::mat4 pipeRight2 = glm::mat4(1.0f);
        // TUBI DESTRA FIXATI: X a 4.3f
        pipeRight2 = glm::translate(pipeRight2, glm::vec3(4.3f, 1.5f, 1.8f)); 
        pipeRight2 = glm::scale(pipeRight2, glm::vec3(0.04f, 1.2f, 0.04f)); 
        envShader.setMat4("model", pipeRight2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // Tubi metallici (Sinistra)
        envShader.setVec3("albedo", glm::vec3(0.02f, 0.02f, 0.02f)); 
        envShader.setFloat("metallic", 0.9f);
        envShader.setFloat("roughness", 0.6f); 
        
        // TUBI SINISTRA FIXATI: Sprofondati nel muro a Z = -3.95f
        glm::mat4 pipe1 = glm::mat4(1.0f);
        pipe1 = glm::translate(pipe1, glm::vec3(-1.6f, 1.3f, -3.95f)); 
        pipe1 = glm::scale(pipe1, glm::vec3(0.04f, 1.8f, 0.04f)); 
        envShader.setMat4("model", pipe1);
        glBindVertexArray(cylVAO);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 pipe2 = glm::mat4(1.0f);
        pipe2 = glm::translate(pipe2, glm::vec3(0.2f, 1.3f, -3.95f)); 
        pipe2 = glm::scale(pipe2, glm::vec3(0.04f, 1.8f, 0.04f)); 
        envShader.setMat4("model", pipe2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- PIANI IN LEGNO ---
        envShader.setVec3("albedo", glm::vec3(0.35f, 0.22f, 0.12f)); 
        envShader.setFloat("metallic", 0.0f);
        envShader.setFloat("roughness", 0.85f); 
        
        // MENSOLA SINISTRA ALTA
        glm::mat4 shelf1 = glm::mat4(1.0f);
        shelf1 = glm::translate(shelf1, glm::vec3(-0.6f, 1.6f, -3.8f)); 
        shelf1 = glm::scale(shelf1, glm::vec3(2.5f, 1.12f, 0.6f)); 
        envShader.setMat4("model", shelf1);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // MENSOLA SINISTRA BASSA
        glm::mat4 shelf2 = glm::mat4(1.0f);
        shelf2 = glm::translate(shelf2, glm::vec3(-0.6f, 0.9f, -3.8f)); 
        shelf2 = glm::scale(shelf2, glm::vec3(2.5f, 1.12f, 0.6f)); 
        envShader.setMat4("model", shelf2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // MENSOLA DESTRA FIXATA: X a 4.3f
        glm::mat4 shelfRight = glm::mat4(1.0f);
        shelfRight = glm::translate(shelfRight, glm::vec3(4.3f, 1.2f, 1.0f)); 
        shelfRight = glm::scale(shelfRight, glm::vec3(1.0f, 1.12f, 2.5f)); 
        envShader.setMat4("model", shelfRight);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // =========================================================
        // TAZZINE (Più grandi, scala +30% circa per proporzione con Moka)
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.9f, 0.9f, 0.85f)); 
        envShader.setFloat("metallic", 0.05f); 
        envShader.setFloat("roughness", 0.2f); 
        
        // --- 1. TAZZINA IN ALTO A SINISTRA ---
        glm::mat4 cup3 = glm::mat4(1.0f);
        // Y alzata a 1.75f per compensare la dimensione maggiore
        cup3 = glm::translate(cup3, glm::vec3(-1.5f, 1.75f, -3.75f)); 
        // SCALA AUMENTATA: X/Z passano da 0.15f a 0.20f. Y passa da 5.0f a 6.5f
        cup3 = glm::scale(cup3, glm::vec3(0.20f, 6.5f, 0.20f));      
        envShader.setMat4("model", cup3);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 handle3 = glm::mat4(1.0f);
        // Manico spostato leggermente più in fuori (X) e più in alto (Y)
        handle3 = glm::translate(handle3, glm::vec3(-1.34f, 1.82f, -3.75f)); 
        handle3 = glm::rotate(handle3, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Manico ispessito proporzionalmente
        handle3 = glm::scale(handle3, glm::vec3(0.08f, 1.8f, 0.08f)); 
        envShader.setMat4("model", handle3);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- 2. TAZZINA BASSA A SINISTRA (Freccia rossa) ---
        glm::mat4 cup1 = glm::mat4(1.0f);
        cup1 = glm::translate(cup1, glm::vec3(-0.2f, 1.05f, -3.75f)); // Y alzata a 1.05f
        cup1 = glm::scale(cup1, glm::vec3(0.20f, 6.5f, 0.20f)); 
        envShader.setMat4("model", cup1);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 handle1 = glm::mat4(1.0f);
        handle1 = glm::translate(handle1, glm::vec3(-0.04f, 1.12f, -3.75f));
        handle1 = glm::rotate(handle1, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        handle1 = glm::scale(handle1, glm::vec3(0.08f, 1.8f, 0.08f)); 
        envShader.setMat4("model", handle1);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- 3. TAZZINA BASSA CENTRALE ---
        glm::mat4 cup2 = glm::mat4(1.0f);
        cup2 = glm::translate(cup2, glm::vec3(0.4f, 1.05f, -3.75f)); 
        cup2 = glm::scale(cup2, glm::vec3(0.20f, 6.5f, 0.20f)); 
        envShader.setMat4("model", cup2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 handle2 = glm::mat4(1.0f);
        handle2 = glm::translate(handle2, glm::vec3(0.56f, 1.12f, -3.75f));
        handle2 = glm::rotate(handle2, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        handle2 = glm::scale(handle2, glm::vec3(0.08f, 1.8f, 0.08f)); 
        envShader.setMat4("model", handle2);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- 4. TAZZINA DESTRA 1 (Sulla mensola FIXATA: X a 4.3f) ---
        glm::mat4 cup4 = glm::mat4(1.0f);
        cup4 = glm::translate(cup4, glm::vec3(4.3f, 1.35f, 0.4f)); 
        cup4 = glm::scale(cup4, glm::vec3(0.20f, 6.5f, 0.20f)); 
        envShader.setMat4("model", cup4);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 handle4 = glm::mat4(1.0f);
        handle4 = glm::translate(handle4, glm::vec3(4.3f, 1.42f, 0.24f)); 
        handle4 = glm::rotate(handle4, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        handle4 = glm::scale(handle4, glm::vec3(0.08f, 1.8f, 0.08f)); 
        envShader.setMat4("model", handle4);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // --- 5. TAZZINA DESTRA 2 (FIXATA: X a 4.3f) ---
        glm::mat4 cup5 = glm::mat4(1.0f);
        cup5 = glm::translate(cup5, glm::vec3(4.3f, 1.35f, 1.0f)); 
        cup5 = glm::scale(cup5, glm::vec3(0.20f, 6.5f, 0.20f)); 
        envShader.setMat4("model", cup5);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        glm::mat4 handle5 = glm::mat4(1.0f);
        handle5 = glm::translate(handle5, glm::vec3(4.3f, 1.42f, 0.84f));
        handle5 = glm::rotate(handle5, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        handle5 = glm::scale(handle5, glm::vec3(0.08f, 1.8f, 0.08f)); 
        envShader.setMat4("model", handle5);
        glDrawArrays(GL_TRIANGLES, 0, cylVertexCount);

        // =========================================================
        // 2E. ACCENTO DECORATIVO (Boiserie) - CORRETTA
        // =========================================================
        envShader.setVec3("albedo", glm::vec3(0.78f, 0.72f, 0.62f));  
        envShader.setFloat("metallic", 0.0f);
        envShader.setFloat("roughness", 1.0f);
        
        glm::mat4 accentWall = glm::mat4(1.0f);
        // FIX: X rimessa a 0.0f. La boiserie ora è perfettamente centrata su tutta la parete di fondo!
        // Se vuoi alzarla o abbassarla sul muro, cambia la Y (attualmente 1.2f).
        accentWall = glm::translate(accentWall, glm::vec3(0.0f, 1.2f, -3.98f)); 
        accentWall = glm::rotate(accentWall, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // SPESSORE FASCIA: Vuoi la striscia più spessa/alta? Aumenta la Y nello scale (ora 0.3f, prova 0.5f).
        accentWall = glm::scale(accentWall, glm::vec3(1.0f, 0.3f, 1.0f));
        envShader.setMat4("model", accentWall);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

       // =========================================================
        // 3. RENDER PARTICELLE E LOGICA CICLICA
        // =========================================================
        
        // --- IL TIMER DELLA MOKA ---
        float cycleDuration = 10.0f; // Il ciclo dura 10 secondi in totale
        float cycleTime = fmod(currentFrame, cycleDuration); 
        
        bool emitSteam = false;
        bool emitCoffee = false;
        
        if (cycleTime < 4.0f) {
            emitSteam = true; // Primi 4 sec: Vapore
        } 
        else if (cycleTime < 8.0f) {
            emitCoffee = true; // Da 4s a 8s: Caffè
        } 

        // Passiamo i flag agli update
        fireSystem.Update(deltaTime, true);
        steamSystem.Update(deltaTime, emitSteam);
        coffeeSystem.Update(deltaTime, emitCoffee);

        particleShader.use();
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        
        glDepthMask(GL_FALSE); 
        fireSystem.Draw(particleShader);
        steamSystem.Draw(particleShader);
        coffeeSystem.Draw(particleShader);
        glDepthMask(GL_TRUE); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //audioManager.Shutdown();
    
    glfwTerminate();
    return 0;
}