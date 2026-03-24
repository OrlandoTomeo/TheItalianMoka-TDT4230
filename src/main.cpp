#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.hpp" // Il tuo file che ora VS Code riconoscerà!

// Impostazioni finestra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    // 1. Inizializza GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0); // Requisito minimo del progetto
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Crea la finestra
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "The Italian Moka Ritual", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 3. Inizializza GLAD prima di chiamare qualsiasi funzione OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Qui in futuro caricheremo gli shader:
    Shader mokaShader("shaders/moka.vert", "shaders/moka.frag");

    // 4. Ciclo di rendering (Finestra che rimane aperta)
    while (!glfwWindowShouldClose(window)) {
        // Input (es. premi ESC per uscire)
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Rendering: Pulisce lo schermo con un colore di sfondo
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Scambia i buffer e interroga gli eventi
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 5. Pulizia finale
    glfwTerminate();
    return 0;
}