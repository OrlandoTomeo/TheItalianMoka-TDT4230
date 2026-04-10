#version 430 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform vec3 cameraPos;
uniform samplerCube skybox;

// Variabili per la luce del fuoco passate dal main.cpp
uniform vec3 firePos;
uniform vec3 fireColor;
uniform float fireConstant;
uniform float fireLinear;
uniform float fireQuadratic;

const float PI = 3.14159265359;

// Parametri Alluminio Satinato Moka
const vec3  albedo    = vec3(0.7, 0.7, 0.7); 
const float metallic  = 1.0;
const float roughness = 0.45; 

// Funzioni PBR
float DistributionGGX(vec3 N, vec3 H, float a) {
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH*NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySmith(float NdotV, float NdotL, float k) {
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ----------------------------------------------------
    // 1. LUCE AMBIENTALE (Riflessi Skybox)
    // ----------------------------------------------------
    vec3 fresnelAmbient = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 envColor = textureLod(skybox, R, roughness * 7.0).rgb;
    vec3 ambient = envColor * fresnelAmbient * 0.6; 

    // ----------------------------------------------------
    // 2. LUCE DINAMICA (Fuoco Sotto la Moka)
    // ----------------------------------------------------
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    
    // Attenuazione della luce (più ci allontaniamo dal fuoco, meno luce fa)
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (fireConstant + fireLinear * distance + fireQuadratic * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    // Calcolo PBR per la luce del fuoco
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(max(dot(N, V), 0.0), max(dot(N, L), 0.0), roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        
    vec3 fireIllumination = (kD * albedo / PI + specular) * radiance * NdotL;

    // ----------------------------------------------------
    // COMPOSIZIONE FINALE
    // ----------------------------------------------------
    vec3 color = ambient + fireIllumination;
    
    // Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}