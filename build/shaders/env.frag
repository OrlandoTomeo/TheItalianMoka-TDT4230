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

// Noise patterns
float hash(vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }

float perlin(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float n00 = hash(i);
    float n10 = hash(i + vec2(1.0, 0.0));
    float n01 = hash(i + vec2(0.0, 1.0));
    float n11 = hash(i + vec2(1.0, 1.0));
    float nx0 = mix(n00, n10, f.x);
    float nx1 = mix(n01, n11, f.x);
    return mix(nx0, nx1, f.y);
}

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 finalAlbedo = albedo;

    // --- 1. TESSERE CUCINA ---
    if (!isCastIron && abs(N.y) < 0.5) {
        if (WorldPos.y > -1.55 && WorldPos.y < 0.2) {
            float tileSize = 0.4;         
            float edgeThickness = 0.025; 
            
            vec2 grid = fract(vec2(WorldPos.x + WorldPos.z, WorldPos.y) / tileSize);
            float mask = step(edgeThickness, grid.x) * step(edgeThickness, grid.y);
            
            float tileNoise = perlin(vec2(WorldPos.x + WorldPos.z, WorldPos.y) * 5.0);
            vec3 baseTile = finalAlbedo * (0.95 + tileNoise * 0.05);
            vec3 groutColor = vec3(0.12, 0.12, 0.12); 
            finalAlbedo = mix(groutColor, baseTile, mask);
        } 
        else if (WorldPos.y >= 0.2 && WorldPos.y < 0.23) {
            finalAlbedo = vec3(0.18, 0.15, 0.12);
        }
        else if (WorldPos.y <= -1.55) {
            finalAlbedo = vec3(0.08, 0.08, 0.08); 
        }
    }

    // --- 2. EFFECT GHISA AGGRESSIVA ---
    if (isCastIron) {
        vec3 noiseVec = vec3(
            perlin(WorldPos.xz * 150.0) - 0.5,
            0.0,
            perlin(WorldPos.xz * 150.0 + vec2(10.0)) - 0.5
        );
        N = normalize(N + noiseVec * 0.25); 
    }

    // --- 3. ILLUMINAZIONE PBR ---
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    vec3 diffuse = finalAlbedo * (1.0 - metallic) * NdotL;
    
    float roughSq = roughness * roughness;
    roughSq *= roughSq; 
    float spec = pow(max(NdotH, 0.0), mix(2.0, 256.0, 1.0 - roughness));
    vec3 specular = radiance * spec * mix(vec3(0.04), finalAlbedo, metallic);

    // --- REFLECTIONS AMBIENTALI ---
    vec3 R = reflect(-V, N);
    vec3 envReflect = texture(skybox, R).rgb;
    float envIntensity = (1.0 - roughness) * metallic;
    vec3 ambientSpec = envReflect * mix(vec3(0.04), finalAlbedo, metallic) * envIntensity;

    float upwardNormal = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 topLightColor = vec3(0.75, 0.85, 0.95);
    vec3 topLight = finalAlbedo * topLightColor * upwardNormal * 0.85; 
    vec3 ambientDiff = (finalAlbedo * 0.07) + topLight; 

    vec3 finalColor = ambientDiff + ambientSpec + (diffuse + specular) * radiance;
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}