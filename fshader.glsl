#version 120

varying vec3 fragColor;

void main() {
    //define sand color
    vec3 sandColor = vec3(0.96, 0.87, 0.7); // Sand color

    //calculate sand texture based on screen coordinates
    float sandTexture = max(0.0, gl_FragCoord.y / 400.0); // Adjust the divisor to control the sand texture height

    //apply shading
    vec3 shadedColor = sandColor * (1.0 - sandTexture); // Darken the sand color towards the bottom of the screen

    //output final color
    gl_FragColor = vec4(shadedColor, 1.0);
}
