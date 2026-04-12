#version 430 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;  // Position from light's perspective

uniform vec3 cameraPos;
uniform vec3 firePos;
uniform vec3 fireColor;
uniform samplerCube skybox;
uniform sampler2D shadowMap;  // Depth texture dalla pass 1

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform bool isCastIron;

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

// === SHADOW CALCULATION (PCF - Percentage Closer Filtering) ===
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Clamp coordinates to valid shadow map range
    if(projCoords.z > 1.0)
        return 0.0;  // Outside light frustum = no shadow
    
    // Get closest depth value from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Calculate bias (prevents shadow acne)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // PCF (Percentage Closer Filtering) - soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // Sample 9 points around the current position
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // Smooth transition at edges
    float edgeFade = smoothstep(0.0, 0.1, projCoords.x) * 
                    smoothstep(1.0, 0.9, projCoords.x) *
                    smoothstep(0.0, 0.1, projCoords.y) *
                    smoothstep(1.0, 0.9, projCoords.y);
    shadow = mix(0.0, shadow, edgeFade);
    
    return shadow;
}

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 finalAlbedo = albedo;

    // --- 1. TEXTURE TILES ---
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

    // --- 2. CAST IRON TEXTURE ---
    if (isCastIron) {
        vec3 noiseVec = vec3(
            perlin(WorldPos.xz * 150.0) - 0.5,
            0.0,
            perlin(WorldPos.xz * 150.0 + vec2(10.0)) - 0.5
        );
        N = normalize(N + noiseVec * 0.25); 
    }

    // --- 3. PBR LIGHTING ---
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.020 * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    
    // === SHADOW FROM DEPTH MAP ===
    float shadow = ShadowCalculation(FragPosLightSpace, N, L);
    
    // Apply shadow to direct lighting
    float shadowFactor = 1.0 - (shadow * 0.8);  // 80% shadow intensity, 20% ambient in shadow
    radiance *= shadowFactor;
    
    // DIFFUSE
    vec3 diffuse = finalAlbedo * (1.0 - metallic) * NdotL;
    
    // SPECULAR WITH METAL ENHANCEMENT
    float roughSq = roughness * roughness;
    roughSq *= roughSq; 
    float spec = pow(max(NdotH, 0.0), mix(2.0, 256.0, 1.0 - roughness));
    float specularIntensity = mix(0.5, 2.5, metallic);
    vec3 specular = radiance * spec * mix(vec3(0.04), finalAlbedo, metallic) * specularIntensity;

    // AMBIENT REFLECTIONS
    vec3 R = reflect(-V, N);
    vec3 envReflect = texture(skybox, R).rgb;
    float envIntensity = (1.0 - roughness) * metallic;
    vec3 ambientSpec = envReflect * mix(vec3(0.04), finalAlbedo, metallic) * envIntensity * 0.6;

    // AMBIENT DIFFUSE
    float upwardNormal = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 topLightColor = vec3(0.35, 0.40, 0.45);
    vec3 topLight = finalAlbedo * topLightColor * upwardNormal * 0.6;
    
    float downwardNormal = max(dot(N, vec3(0.0, -1.0, 0.0)), 0.0);
    vec3 bottomLightColor = vec3(0.8, 0.3, 0.1);
    vec3 bottomLight = finalAlbedo * bottomLightColor * downwardNormal * 0.3;
    
    // In shadow, boost ambient to prevent complete darkness
    vec3 ambientDiff = (finalAlbedo * (0.08 + shadow * 0.15)) + topLight + bottomLight;

    // FINAL COMPOSITION
    vec3 finalColor = ambientDiff + ambientSpec + (diffuse + specular) * radiance;
    
    finalColor = finalColor / (finalColor + vec3(0.9));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}
