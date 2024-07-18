#version 450

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;

layout (early_fragment_tests) in;

void main() {
    outPosition = vec4(fragPosition, 1.0);
    outNormal = vec4(0.5 * normalize(fragNormal) + 0.5, 1.0);
}