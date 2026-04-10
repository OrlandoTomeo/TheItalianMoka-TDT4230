#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 camPos;
// Lasciamo le uniform preparate per dopo
uniform samplerCube skybox; 
uniform float roughness;    
uniform float metallic;

void main() {
    // DEBUG TEMPORANEO: Usiamo le normali per dare colore al cubo
    // Mappiamo le normali da [-1, 1] a [0, 1] per ottenere colori validi
    vec3 debugColor = normalize(Normal) * 0.5 + 0.5;
    
    FragColor = vec4(debugColor, 1.0);
}