#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    // Togliamo la traslazione dalla matrice view trasformandola in mat3 e poi di nuovo in mat4
    mat4 viewRotation = mat4(mat3(view)); 
    vec4 pos = projection * viewRotation * vec4(aPos, 1.0);
    
    // Il trucco .xyww forza la Skybox a stare dietro a tutto (Z = 1.0)
    gl_Position = pos.xyww;
}