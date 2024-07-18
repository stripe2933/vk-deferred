#version 450

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;

layout (early_fragment_tests) in;

void main() {
    outPosition = fragPosition;
    outNormal = 0.5 * normalize(fragNormal) + 0.5;
}