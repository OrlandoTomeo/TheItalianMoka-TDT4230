#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;  // MVP da POV della luce

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
