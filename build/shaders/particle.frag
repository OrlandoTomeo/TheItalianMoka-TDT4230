#version 430 core
out vec4 FragColor;
in vec4 ParticleColor;

void main() {
    // Calcola la distanza dal centro del punto (da -1 a 1)
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    float dist = dot(circCoord, circCoord);
    
    // Taglia i pixel fuori dal cerchio
    if(dist > 1.0) discard;

    // Sfuma i bordi per un effetto fumo/fuoco morbido
    float alpha = ParticleColor.a * (1.0 - dist);
    FragColor = vec4(ParticleColor.rgb, alpha);
}