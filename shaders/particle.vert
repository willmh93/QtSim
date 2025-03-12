#version 330 core

layout(location = 0) in vec2 position;

uniform mat4 transform;         // Combined projection * model matrix
uniform float particleSize;     // Scaled particle size

void main() {
    gl_Position = transform * vec4(position, 0.0, 1.0);
    gl_PointSize = particleSize;  // This is in screen pixels
}
