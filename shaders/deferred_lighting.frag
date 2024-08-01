#version 450

const float specularStrength = 0.5;
const vec3 materialColor = vec3(0.5);

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;

layout (location = 0) flat in vec3 fragInstancePosition;
layout (location = 1) flat in float fragInstanceRadius;
layout (location = 2) flat in vec3 fragInstanceColor;

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

layout (early_fragment_tests) in;

float square(float x) {
    return x * x;
}

void main(){
    vec3 fragPosition = subpassLoad(inputPosition).xyz;
    float lightDistance = length(fragInstancePosition - fragPosition);
    if (lightDistance > fragInstanceRadius) {
        outColor = vec3(0.0);
        return;
    }

    float attenuation = square(lightDistance / fragInstanceRadius - 1.0);
    vec3 lightColor = fragInstanceColor * attenuation;
    vec3 fragNormal = 2.0 * subpassLoad(inputNormal).xyz - 1.0;

    // Diffuse.
    vec3 lightDir = normalize(fragInstancePosition - fragPosition);
    float diff = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular.
    vec3 viewDir = normalize(pc.viewPosition - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(fragNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    outColor = (diffuse + specular) * materialColor;
}