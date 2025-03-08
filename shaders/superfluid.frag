#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform float iTime;        // Pass the elapsed time (in seconds)
uniform vec2 iResolution;   // The viewport resolution

//
// A 2D simplex noise function (inspired by Inigo Quilez)
//
vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 permute(vec3 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}
float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                       -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0/41.0
    // First corner
    vec2 i  = floor(v + dot(v, vec2(C.y, C.y)));
    vec2 x0 = v - i + dot(i, vec2(C.x, C.x));

    // Other corners
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 x1 = x0 - i1 + C.x;
    vec2 x2 = x0 - 1.0 + 2.0 * C.x;

    // Permutations
    i = mod289(i);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
                    + i.x + vec3(0.0, i1.x, 1.0));

    // Gradients: 41 points uniformly over a unit circle, mapped onto a diamond.
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
    m = m * m;
    m = m * m;

    vec3 x = 2.0 * fract(p * C.w) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.y = a0.y * x1.x + h.y * x1.y;
    g.z = a0.z * x2.x + h.z * x2.y;
    return 130.0 * dot(m, g);
}

void main(){
    // Normalize fragment coordinates (-1 to 1)
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;

    float t = iTime * 0.5;

    // Convert cartesian coords to polar for swirling
    float angle = atan(uv.y, uv.x);
    float radius = length(uv);

    // Swirling distortion
    float swirl = sin(10.0 * radius - t * 5.0 + angle * 4.0);

    // Combine noise and swirl for a dynamic fluid effect
    float n = snoise(uv * 3.0 + vec2(t));
    float intensity = smoothstep(0.2, 0.5, radius + 0.3 * n + 0.2 * swirl);

    // Define a color gradient (mixing deep purple and blue)
    vec3 colorA = vec3(0.1, 0.0, 0.2);
    vec3 colorB = vec3(0.0, 0.5, 1.0);
    vec3 col = mix(colorA, colorB, intensity);

    // Add subtle noise highlights
    col += 0.1 * vec3(n);

    FragColor = vec4(col, 1.0);
}
