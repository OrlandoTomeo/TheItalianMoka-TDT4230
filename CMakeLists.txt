# Richiediamo la 3.14 perché ci serve per il comando FetchContent
cmake_minimum_required(VERSION 3.14)
project(MokaProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Cerca OpenGL
find_package(OpenGL REQUIRED)

# -------------------------------------------------------------------
# 2. LA MAGIA: CMake scarica e compila GLFW in automatico!
# -------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glfw
    # Scarichiamo uno .zip ufficiale così non hai bisogno di avere Git installato
    URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
)
# Spegniamo la compilazione di test e documentazione per fare in fretta
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
# -------------------------------------------------------------------

# 3. Crea il tuo eseguibile
add_executable(MokaProject
    src/main.cpp
    src/shader.cpp
    src/glad.c
)

# 4. Includi le cartelle (per GLAD e GLM)
# Nota: Non ci serve più preoccuparci degli header di GLFW, ci pensa CMake!
target_include_directories(MokaProject PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/include
)

# 5. Link delle librerie
# Ora linkiamo direttamente il target 'glfw' che CMake ha appena costruito per te.
# Si occuperà lui di aggiungere in automatico gdi32, user32, ecc.
target_link_libraries(MokaProject PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
)

# 6. Copia gli shader nella cartella build
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})