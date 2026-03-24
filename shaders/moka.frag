#version 430 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// uniforms per le texture e la luce
uniform vec3 camPos;
uniform samplerCube skybox; // Per i riflessi (Fase 2)
uniform float roughness;    // Per le micro-faccette (Fase 3)
uniform float metallic;

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    
    // Vettore di riflessione per la cubemap
    vec3 R = reflect(-V, N);
    
    // Sample base dalla cubemap
    vec3 reflectionColor = texture(skybox, R).rgb;
    
    // Più avanti qui implementeremo la BRDF di Cook-Torrance per le micro-faccette!
    
    FragColor = vec4(reflectionColor, 1.0);
}