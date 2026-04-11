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

// Load Cubemap (Kept for reflections even if not drawn as background)
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
    
    // Generate mipmaps for roughness calculation
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    // Set parameters using trilinear filtering for mipmaps
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}

int main() {
    // Initialize GLFW and set OpenGL version to 4.3 Core
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Italian Moka Ritual", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    // Load OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // Enable Depth Testing
    glEnable(GL_DEPTH_TEST);

    // Initialize shaders and model
    Shader mokaShader("shaders/moka.vert", "shaders/moka.frag");
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag"); // SHADER PARTICELLE
    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");

    // We still load the cubemap textures to use as Environment Map for PBR reflections
    std::vector<std::string> faces = { "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);

    // --- SETUP PARTICLE SYSTEMS ---
    // Fire: Posizionato perfettamente sotto la base (-0.9)
    ParticleSystem fireSystem(150, glm::vec3(0.0f, -0.85f, 0.0f), true); 
    // Steam: Posizione invariata (hai detto che andava bene)
    ParticleSystem steamSystem(100, glm::vec3(-1.20f, 0.75f, 0.0f), false); 

    // Time variables for smooth animations
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate DeltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input handling
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);
        
        // Clear screen with elegant dark grey studio background
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---------------------------------------------------------
        // CAMERA SETUP (TELECAMERA ORBITANTE RIPARATA)
        // ---------------------------------------------------------
        float radius = 4.5f;
        float camSpeed = 0.8f; 
        float camX = sin(currentFrame * camSpeed) * radius;
        float camZ = cos(currentFrame * camSpeed) * radius;
        glm::vec3 cameraPos = glm::vec3(camX, 2.5f, camZ); 
        
        glm::vec3 target = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // --- DYNAMIC FIRE LIGHT ---
        glm::vec3 firePos = glm::vec3(0.0f, -0.8f, 0.0f); 
        float flickerIntensity = 0.8f + (sin(currentFrame * 15.0f) + cos(currentFrame * 21.0f)) * 0.1f;
        glm::vec3 fireColor = glm::vec3(5.0f, 2.0f, 0.2f) * flickerIntensity;

        // ---------------------------------------------------------
        // 1. RENDER MOKA PBR
        // ---------------------------------------------------------
        mokaShader.use();
        mokaShader.setVec3("cameraPos", cameraPos);
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);
        
        mokaShader.setVec3("firePos", firePos);
        mokaShader.setVec3("fireColor", fireColor);
        mokaShader.setFloat("fireConstant", 1.0f);
        mokaShader.setFloat("fireLinear", 0.09f);
        mokaShader.setFloat("fireQuadratic", 0.032f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        mokaShader.setInt("skybox", 0);

        // --- MOKA TRANSFORMATIONS ---
       glm::mat4 model = glm::mat4(1.0f);

        
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, 0.0f));

        model = glm::rotate(model, glm::radians(16.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(210.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. SCALA
        model = glm::scale(model, glm::vec3(1.2f));

        mokaShader.setMat4("model", model);
        mokaBialetti.Draw(mokaShader);

        // ---------------------------------------------------------
        // 2. RENDER PARTICLES (Fire and Steam)
        // ---------------------------------------------------------
        fireSystem.Update(deltaTime);
        steamSystem.Update(deltaTime);

        particleShader.use();
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        
        glDepthMask(GL_FALSE); 
        fireSystem.Draw(particleShader);
        steamSystem.Draw(particleShader);
        glDepthMask(GL_TRUE); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    glfwTerminate();
    return 0;
}