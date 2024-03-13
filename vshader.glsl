#version 120

attribute vec2 inVertex;
attribute vec3 inColor;

varying vec3 fragColor;

void main() {
    gl_Position = vec4(inVertex, 0.0, 1.0);
    fragColor = inColor;
}
