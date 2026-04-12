cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0135 NEW)

# Soluzione all'errore di compatibilità GLM
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

project(MokaProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
include(FetchContent)

# -------------------------------------------------------------------
# 2. GLFW (Finestra e Input)
# -------------------------------------------------------------------
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# -------------------------------------------------------------------
# 3. GLM (Matematica 3D)
# -------------------------------------------------------------------
FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
)
FetchContent_MakeAvailable(glm)

# -------------------------------------------------------------------
# 4. ASSIMP (Ora usa il file LOCALE per evitare cali di connessione!)
# -------------------------------------------------------------------
FetchContent_Declare(
    assimp
    URL ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp-5.0.1.zip
)

# 1. LA TOPPA PER L'ERRORE ZIP.C (Impedisce il conflitto di ssize_t)
add_compile_definitions(_SSIZE_T_DEFINED)

# 2. SPEGNIAMO LE COSE INUTILI
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE) 
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

# 3. L'ARMA SEGRETA: Compiliamo SOLO i due formati che ci servono!
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# -------------------------------------------------------------------
# 5. Crea l'eseguibile
# -------------------------------------------------------------------
add_executable(MokaProject
    src/main.cpp
    #src/shader.cpp
    #src/audioManager.cpp
    src/glad.c
    src/stb_image.cpp
    src/Shader.h        
    src/Model.h
    #src/Camera.h
    #src/PointLight.h
    #src/ParticleSystem.h
)

target_include_directories(MokaProject PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/include
    #${CMAKE_CURRENT_SOURCE_DIR}/external/include/utils
)

target_link_directories(MokaProject PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/lib)

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
#
#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/lib/libOpenAL32.a")
#    set(OPENAL_LIB "${CMAKE_CURRENT_SOURCE_DIR}/external/lib/libOpenAL32.a")
#elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/lib/OpenAL32.lib")
#    set(OPENAL_LIB "${CMAKE_CURRENT_SOURCE_DIR}/external/lib/OpenAL32.lib")
#else()
#    message(FATAL_ERROR "Nessuna OpenAL trovata in external/lib!")
#endif()
#
#message(STATUS ">>> OpenAL usata: ${OPENAL_LIB}")
#
#if(NOT OPENAL_LIB)
#    message(FATAL_ERROR "OpenAL non trovata in external/lib! Controlla che ci sia OpenAL32.lib o libOpenAL32.a")
#endif()
#
#message(STATUS "OpenAL trovata: ${OPENAL_LIB}")

target_link_libraries(MokaProject PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    glm::glm
    assimp
    #${CMAKE_CURRENT_SOURCE_DIR}/external/lib/libOpenAL32.dll.a
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})