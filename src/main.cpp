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
    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");

    // We still load the cubemap textures to use as Environment Map for PBR reflections
    std::vector<std::string> faces = { "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input handling
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);
        
        // Clear screen with elegant dark grey studio background
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---------------------------------------------------------
        // CAMERA SETUP
        // ---------------------------------------------------------
        // Lowered Y to 1.0 and moved slightly closer (-4.5) for a majestic framing
        glm::vec3 cameraPos = glm::vec3(0.0f, 2.5f, -4.5f); 
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // --- DYNAMIC FIRE LIGHT ---
        glm::vec3 firePos = glm::vec3(0.0f, 0.0f, 2.0f); 
        
        float flickerIntensity = 0.8f + (sin((float)glfwGetTime() * 15.0f) + cos((float)glfwGetTime() * 21.0f)) * 0.1f;
        
        // Esagero i valori del colore (15, 5, 1) per creare un bagliore fortissimo
        glm::vec3 fireColor = glm::vec3(15.0f, 5.0f, 1.0f) * flickerIntensity;

        // 1. Render Moka PBR
        mokaShader.use();
        mokaShader.setVec3("cameraPos", cameraPos);
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);
        
        // Pass fire light parameters to the shader
        mokaShader.setVec3("firePos", firePos);
        mokaShader.setVec3("fireColor", fireColor);
        mokaShader.setFloat("fireConstant", 1.0f);
        mokaShader.setFloat("fireLinear", 0.09f);
        mokaShader.setFloat("fireQuadratic", 0.032f);

        // Bind the cubemap for reflections
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        mokaShader.setInt("skybox", 0);

        // ---------------------------------------------------------
        // MOKA TRANSFORMATIONS
        // ---------------------------------------------------------
        glm::mat4 model = glm::mat4(1.0f);

        // 1. FIX Z-up → Y-up (QUESTO È IL PASSO CHIAVE)
        model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1, 0, 0));

        // 2. ORIENTAMENTO (manico a sinistra)
        model = glm::rotate(model, glm::radians(210.0f), glm::vec3(0, 1, 0));

        // 3. POSIZIONE
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, 0.0f));

        // 4. SCALA
        model = glm::scale(model, glm::vec3(1.2f));

        mokaShader.setMat4("model", model);
        mokaBialetti.Draw(mokaShader);
        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    glfwTerminate();
    return 0;
}