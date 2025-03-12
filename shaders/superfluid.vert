#version 330 core
layout(location = 0) in vec2 aPos;

out vec2 TexCoord;
out vec3 WorldPos;

uniform mat4 transform;
uniform mat4 projection;

void main() {
    // Convert 2D aPos to 3D by adding a z = 0
    vec4 worldPosition = transform * vec4(aPos, 0.0, 1.0); 
    WorldPos = worldPosition.xyz;  // Store world coordinates BEFORE projection

    gl_Position = transform * projection * vec4(aPos, 0.0, 1.0);  // Skip projection

    //gl_Position = projection * worldPosition; // Apply orthographic projection
}
