#version 430 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform vec3 firePos;
uniform vec3 fireColor;
uniform samplerCube skybox;

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform bool isCastIron;

// Funzione matematica per generare rumore (noise)
float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - WorldPos);

    // 1. EFFETTO GHISA PROCEDURALE
    if (isCastIron) {
        float noise = hash(floor(WorldPos.xz * 300.0));
        N = normalize(N + vec3(noise * 0.15, 0.0, noise * 0.15));
    }

    // 2. ILLUMINAZIONE DINAMICA DEL FUOCO
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * (1.0 - metallic) * NdotL;
    
    // Glossiness basata sulla roughness
    float spec = pow(max(dot(N, H), 0.0), mix(2.0, 256.0, 1.0 - roughness));
    vec3 specular = radiance * spec * mix(vec3(0.04), albedo, metallic);

    // 3. RIFLESSI DELL'AMBIENTE (Skybox)
    vec3 R = reflect(-V, N);
    vec3 envReflect = texture(skybox, R).rgb;
    vec3 ambientSpec = envReflect * mix(vec3(0.04), albedo, metallic) * (1.0 - roughness);

    // ----------------------------------------------------
    // 4. LUCE ZENITALE (Hemispheric Light)
    // ----------------------------------------------------
    // Quanto la normale punta verso l'alto (Y positivo)
    float upwardNormal = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    
    // Colore morbido dall'alto (leggermente azzurro/freddo per contrastare il fuoco caldo)
    vec3 topLightColor = vec3(0.6, 0.65, 0.7); 
    vec3 topLight = albedo * topLightColor * upwardNormal * 0.5; // Moltiplicatore di intensità
    
    // La luce base minima più la luce dall'alto
    vec3 ambientDiff = (albedo * 0.05) + topLight; 

    // Composizione Finale
    vec3 finalColor = ambientDiff + ambientSpec + (diffuse + specular) * radiance;

    // Tone mapping e Correzione Gamma
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}