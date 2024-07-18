#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inInstancePosition;
layout (location = 2) in float inInstanceRadius;
layout (location = 3) in vec3 inInstanceColor;

layout (location = 0) flat out vec3 fragInstancePosition;
layout (location = 1) flat out float fragInstanceRadiusSq;
layout (location = 2) flat out vec3 fragInstanceColor;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

void main(){
    fragInstancePosition = inInstancePosition;
    fragInstanceRadiusSq = inInstanceRadius * inInstanceRadius;
    fragInstanceColor = inInstanceColor;

    gl_Position = pc.projectionView * vec4(inInstanceRadius * inPosition + inInstancePosition, 1.0);
}