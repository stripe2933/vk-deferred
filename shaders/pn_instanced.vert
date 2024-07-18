#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in mat4 instanceTransform;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
} pc;

void main() {
    fragPosition = vec3(instanceTransform * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(instanceTransform))) * inNormal;

    gl_Position = pc.projectionView * vec4(fragPosition, 1.0);
}