#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "shader.hpp"
#include "model.hpp" 

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Moka Showroom", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader mokaShader("shaders/moka.vert", "shaders/moka.frag");

    // CARICHIAMO I DUE MODELLI
    // Assicurati che i nomi corrispondano esattamente a quelli nella cartella!
    Model mokaBialetti("models/bialetti_moka_espresso_maker_1933.glb");
    Model mokaPot("models/moka_pot.glb");

    while (!glfwWindowShouldClose(window)) {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Uno sfondo scuro ed elegante da showroom
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mokaShader.use();

        // TELECAMERA: Ci allontaniamo un po' di più per vederle entrambe
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 6.0f), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        mokaShader.setMat4("view", view);
        mokaShader.setMat4("projection", projection);

        // --- DISEGNO MOKA 1 (Bialetti - Sinistra) ---
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-1.5f, 0.0f, 0.0f)); // Spostata a sinistra
        model1 = glm::rotate(model1, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotazione lenta
        // NOTA SULLA SCALA: Se il modello è gigante o invisibile, modifica questo valore (es: 0.1f o 10.0f)
        model1 = glm::scale(model1, glm::vec3(1.0f, 1.0f, 1.0f)); 
        
        mokaShader.setMat4("model", model1);
        mokaBialetti.Draw(mokaShader);

        // --- DISEGNO MOKA 2 (Generica - Destra) ---
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(1.5f, 0.0f, 0.0f)); // Spostata a destra
        model2 = glm::rotate(model2, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        // NOTA SULLA SCALA: Se i modelli hanno dimensioni diverse, bilanciali qui
        model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 1.0f)); 
        
        mokaShader.setMat4("model", model2);
        mokaPot.Draw(mokaShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}