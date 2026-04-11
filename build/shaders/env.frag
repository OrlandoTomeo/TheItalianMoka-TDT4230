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

float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - WorldPos);
    
    vec3 finalAlbedo = albedo;

    // --- 1. EFFETTO MATTONELLE CUCINA A METÀ MURO ---
    // Se la normale Y è quasi zero, significa che è un muro verticale
    if (!isCastIron && abs(N.y) < 0.5) {
        
        // Se siamo nella parte bassa del muro (es. sotto Y = 0.2), disegniamo le piastrelle
        if (WorldPos.y < 0.2) {
            float tileSize = 0.4;         
            float edgeThickness = 0.03;   
            
            vec2 grid = fract(vec2(WorldPos.x + WorldPos.z, WorldPos.y) / tileSize);
            float mask = step(edgeThickness, grid.x) * step(edgeThickness, grid.y);
            
            vec3 groutColor = vec3(0.1, 0.1, 0.1); 
            finalAlbedo = mix(groutColor, finalAlbedo, mask);
        } 
        // Se siamo esattamente sul confine (tra 0.2 e 0.23), disegniamo un bordino scuro
        else if (WorldPos.y >= 0.2 && WorldPos.y < 0.23) {
            finalAlbedo = vec3(0.15, 0.12, 0.1); // Colore del bordino
        }
        // Sopra 0.23 rimane l'intonaco liscio e pulito!
    }

    // --- 2. EFFETTO GHISA PROCEDURALE ---
    if (isCastIron) {
        float noise = hash(floor(WorldPos.xz * 300.0));
        N = normalize(N + vec3(noise * 0.15, 0.0, noise * 0.15));
    }

    // --- 3. ILLUMINAZIONE ---
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = finalAlbedo * (1.0 - metallic) * NdotL;
    
    float spec = pow(max(dot(N, H), 0.0), mix(2.0, 256.0, 1.0 - roughness));
    vec3 specular = radiance * spec * mix(vec3(0.04), finalAlbedo, metallic);

    vec3 R = reflect(-V, N);
    vec3 envReflect = texture(skybox, R).rgb;
    vec3 ambientSpec = envReflect * mix(vec3(0.04), finalAlbedo, metallic) * (1.0 - roughness);

    float upwardNormal = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 topLightColor = vec3(0.6, 0.65, 0.7); 
    vec3 topLight = finalAlbedo * topLightColor * upwardNormal * 0.5; 
    vec3 ambientDiff = (finalAlbedo * 0.05) + topLight; 

    // Composizione Finale
    vec3 finalColor = ambientDiff + ambientSpec + (diffuse + specular) * radiance;
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}