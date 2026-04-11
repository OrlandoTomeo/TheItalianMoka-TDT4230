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
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
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
    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");

    std::vector<std::string> faces = { "textures/right.jpg", "textures/left.jpg", "textures/top.jpg", "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);

    // --- VARIABILI DEVELOPER MODE (Inizializzate con i tuoi ultimi dati) ---
    glm::vec3 currentFirePos = glm::vec3(-0.262055f, -1.48022f, -0.674087f);
    glm::vec3 currentSteamPos = glm::vec3(-0.167861f, 0.266838f, -0.473491f);
    
    float mokaRotX = 19.4863f;
    float mokaRotY = 196.127f;
    float mokaPosY = -0.8f;

    ParticleSystem fireSystem(150, currentFirePos, true); 
    ParticleSystem steamSystem(100, currentSteamPos, false); 

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
            glfwSetWindowShouldClose(window, true);
        
        // --- CONTROLLI TASTIERA ---
        float moveSpeed = 1.0f * deltaTime; 
        float rotSpeed = 25.0f * deltaTime; // Gradi al secondo per la rotazione
        
        // 1. Muovi FUOCO (Frecce + O/L)
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) currentFirePos.z -= moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) currentFirePos.z += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) currentFirePos.x -= moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) currentFirePos.x += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) currentFirePos.y += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) currentFirePos.y -= moveSpeed;

        // 2. Muovi VAPORE (WASD + R/F)
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) currentSteamPos.z -= moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) currentSteamPos.z += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) currentSteamPos.x -= moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) currentSteamPos.x += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) currentSteamPos.y += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) currentSteamPos.y -= moveSpeed;

        // 3. Muovi MOKA (Rotazioni e Altezza)
        if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) mokaRotX += rotSpeed;
        if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) mokaRotX -= rotSpeed;
        if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) mokaRotY += rotSpeed;
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) mokaRotY -= rotSpeed;
        if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) mokaPosY += moveSpeed;
        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) mokaPosY -= moveSpeed;

        // Stampa i dati aggiornati
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            std::cout << "--- POSITION SAVED ---" << std::endl;
            std::cout << "FIRE : X=" << currentFirePos.x << " Y=" << currentFirePos.y << " Z=" << currentFirePos.z << std::endl;
            std::cout << "STEAM: X=" << currentSteamPos.x << " Y=" << currentSteamPos.y << " Z=" << currentSteamPos.z << std::endl;
            std::cout << "MOKA : RotX=" << mokaRotX << " RotY=" << mokaRotY << " PosY=" << mokaPosY << std::endl;
        }

        fireSystem.setSpawnPosition(currentFirePos);
        steamSystem.setSpawnPosition(currentSteamPos);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- CAMERA ---
        float radius = 4.5f;
        float camSpeed = 0.1f; 
        float camX = sin(currentFrame * camSpeed) * radius;
        float camZ = cos(currentFrame * camSpeed) * radius;
        glm::vec3 cameraPos = glm::vec3(camX, 2.5f, camZ); 
        
        glm::vec3 target = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // --- LUCE FUOCO ---
        float flickerIntensity = 0.8f + (sin(currentFrame * 15.0f) + cos(currentFrame * 21.0f)) * 0.1f;
        glm::vec3 fireColor = glm::vec3(5.0f, 2.0f, 0.2f) * flickerIntensity;

        // --- RENDER MOKA ---
        mokaShader.use();
        mokaShader.setVec3("cameraPos", cameraPos);
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);
        
        // La luce dinamica segue esattamente la posizione centrale dell'anello di fuoco
        mokaShader.setVec3("firePos", currentFirePos);
        mokaShader.setVec3("fireColor", fireColor);
        mokaShader.setFloat("fireConstant", 1.0f);
        mokaShader.setFloat("fireLinear", 0.09f);
        mokaShader.setFloat("fireQuadratic", 0.032f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        mokaShader.setInt("skybox", 0);

        // Applicazione variabili dinamiche alla Moka
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, mokaPosY, 0.0f));
        model = glm::rotate(model, glm::radians(mokaRotX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(mokaRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.2f));

        mokaShader.setMat4("model", model);
        mokaBialetti.Draw(mokaShader);

        // --- RENDER PARTICELLE ---
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
    
    glfwTerminate();
    return 0;
}