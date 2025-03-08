#version 330 core

uniform vec4 glowColor;
uniform float glowStrength;

out vec4 fragColor;

void main() {
    // Center the coordinates (gl_PointCoord ranges 0 to 1)
    vec2 centeredCoord = gl_PointCoord - vec2(0.5, 0.5);

    // Distance from center to edge (normalized)
    float dist = length(centeredCoord) * 2.0;  // Range [0, 1] (from center to corner)

    // Optional: discard outside circle (for sharp cut-off)
    if (dist > 1.0) {
        discard;
    }

    // Exponential falloff for soft glow effect
    float alpha = 1-dist;//exp(-glowStrength * dist * dist);
    if (dist < 0) {
        discard;
    }

    fragColor = vec4(glowColor.rgb, glowColor.a * alpha);
}
