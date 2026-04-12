#version 430 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec3 LocalPos; // RICEVE LA POSIZIONE DAL VERTEX SHADER

uniform vec3 cameraPos;
uniform samplerCube skybox;

// Variabili per la luce del fuoco passate dal main.cpp
uniform vec3 firePos;
uniform vec3 fireColor;
uniform float fireConstant;
uniform float fireLinear;
uniform float fireQuadratic;

const float PI = 3.14159265359;

// Parametri base dell'Alluminio
const vec3  albedoBase    = vec3(0.7, 0.7, 0.7); 
const float metallicBase  = 1.0;
const float roughnessBase = 0.45; 

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

    // ====================================================
    // IL TRUCCO DELLA BACHELITE (Plastica Nera Opaca)
    // ====================================================
    vec3 currentAlbedo = albedoBase;
    float currentMetallic = metallicBase;
    float currentRoughness = roughnessBase;

    // 1. IL POMELLO (Taglio orizzontale in alto)
    // Se colora troppo coperchio: ALZA il numero (es. 1.40)
    // Se lascia la punta argentata: ABBASSA il numero (es. 1.30)
    bool isKnob = LocalPos.y > 1.80; 

    // 2. IL MANICO (Taglio verticale a sinistra)
    // Se colora un pezzo di caldaia: Mettilo più negativo (es. -0.65, -0.70)
    // Se lascia metà manico argentato: Mettilo meno negativo (es. -0.55, -0.50)
    bool isHandle = (LocalPos.x < -0.48) && (LocalPos.y > 0.1); 

    if (isKnob || isHandle) {
        currentAlbedo = vec3(0.02, 0.02, 0.02); // Plastica Nera
        currentMetallic = 0.0;                  // Zero riflessi metallici
        currentRoughness = 0.85;                // Molto opaca, non scivolosa
    }

    // Calcoliamo F0 basandoci sulle nuove variabili
    vec3 F0 = mix(vec3(0.04), currentAlbedo, currentMetallic);

    // ----------------------------------------------------
    // 1. LUCE AMBIENTALE SULLA MOKA (Riflessi + Luce dall'alto)
    // ----------------------------------------------------
    vec3 fresnelAmbient = fresnelSchlick(max(dot(N, V), 0.0), F0);
    
    vec3 envColor = textureLod(skybox, R, currentRoughness * 7.0).rgb;
    // Riportiamo i riflessi a 0.4 (prima li avevamo "uccisi" a 0.1)
    vec3 skyboxReflect = envColor * fresnelAmbient * 0.4; 

    float upwardNormal = max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0);
    vec3 topLightColor = vec3(0.6, 0.65, 0.7); 
    
    vec3 kS_zenith = fresnelSchlick(max(dot(N, vec3(0.0, 1.0, 0.0)), 0.0), F0);
    vec3 kD_zenith = 1.0 - kS_zenith;
    kD_zenith *= 1.0 - currentMetallic; 

    // Aumentiamo la luce diffusa dall'alto (da 0.02 a 0.3)
    vec3 diffuseZenith = (kD_zenith * currentAlbedo) * topLightColor * upwardNormal * 0.3;
    
    vec3 ambient = skyboxReflect + diffuseZenith;
    // ----------------------------------------------------
    // 2. LUCE DINAMICA (Fuoco Sotto la Moka)
    // ----------------------------------------------------
    vec3 L = normalize(firePos - WorldPos);
    vec3 H = normalize(V + L);
    
    float distance = length(firePos - WorldPos);
    float attenuation = 1.0 / (fireConstant + fireLinear * distance + fireQuadratic * (distance * distance));
    vec3 radiance = fireColor * attenuation;

    // Usiamo currentRoughness per la specularità
    float NDF = DistributionGGX(N, H, currentRoughness);   
    float G   = GeometrySmith(max(dot(N, V), 0.0), max(dot(N, L), 0.0), currentRoughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - currentMetallic; // Usiamo currentMetallic

    float NdotL = max(dot(N, L), 0.0);        
    vec3 fireIllumination = (kD * currentAlbedo / PI + specular) * radiance * NdotL;

    // ----------------------------------------------------
    // COMPOSIZIONE FINALE
    // ----------------------------------------------------
    vec3 color = ambient + fireIllumination;
    
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}