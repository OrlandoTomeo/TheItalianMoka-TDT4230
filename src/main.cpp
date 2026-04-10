#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>

#include "stb_image.h"
#include "shader.hpp"
#include "model.hpp"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// =======================================================
// FUNZIONE PER CARICARE LA SKYBOX
// =======================================================
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 4) format = GL_RGBA;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Errore caricamento cubemap alla path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// =======================================================
// VERTICI DEL CUBO DELLA SKYBOX
// =======================================================
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Italian Moka Ritual", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    Shader mokaShader("shaders/moka.vert", "shaders/moka.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces = {
        "textures/right.jpg", "textures/left.jpg", "textures/top.jpg",
        "textures/bottom.jpg", "textures/front.jpg", "textures/back.jpg"
    };

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    
    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    while (!glfwWindowShouldClose(window)) {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- TELECAMERA MODIFICATA ---
        // Posizione: Z = -5.0 (per vedere front.jpg come sfondo) e Y = 1.0 (altezza media)
        glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, -5.0f);
        // Target: Puntiamo a Y = 1.5 (leggermente sopra il centro della moka che è a Y=0)
        glm::vec3 cameraTarget = glm::vec3(0.0f, 1.5f, 0.0f);
        
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // 1. DISEGNO LA MOKA
        mokaShader.use();
        mokaShader.setVec3("cameraPos", cameraPos);
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);
                // --- POSIZIONAMENTO MOKA DEFINITIVO ---
        glm::mat4 model = glm::mat4(1.0f);

        // 1. Spostiamola un po' verso il basso per appoggiarla idealmente sul tavolo
        model = glm::translate(model, glm::vec3(0.0f, -0.8f, 0.0f)); 

        // 2. METTIAMOLA IN PIEDI
        // Se con -90 era sdraiata, significa che il modello è già "Y-up". 
        // Mettiamo 0 gradi (o cancella la riga) per lasciarla dritta.
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 

        // 3. MANICO VERSO SINISTRA
        // Ora che è dritta, usiamo l'asse Y (0,1,0) per farla girare su se stessa.
        // Prova 90.0f per vedere il manico. Se vedi il beccuccio, metti -90.0f.
        model = glm::rotate(model, glm::radians(-125.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 

        // 4. DIMENSIONI
        model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f)); 

        mokaShader.setMat4("model", model);
        mokaBialetti.Draw(mokaShader);

        // 2. DISEGNO LA SKYBOX
        glDepthFunc(GL_LEQUAL);  
        skyboxShader.use();
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}