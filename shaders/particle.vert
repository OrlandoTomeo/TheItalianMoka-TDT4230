#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;

out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    ParticleColor = aColor;
    vec4 viewPos = view * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    
    // Rimpicciolisce le particelle più sono lontane
    gl_PointSize = (aSize * 300.0) / -viewPos.z; 
}