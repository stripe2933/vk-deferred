#version 450

const float specularStrength = 0.5;
const vec3 materialColor = vec3(0.5);

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;

layout (location = 0) flat in vec3 fragInstancePosition;
layout (location = 1) flat in float fragInstanceRadiusSq;
layout (location = 2) flat in vec3 fragInstanceColor;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform PushConstant {
    mat4 projectionView;
    vec3 viewPosition;
} pc;

layout (early_fragment_tests) in;

float length2(vec3 v) {
    return dot(v, v);
}

void main(){
    vec3 fragPosition = subpassLoad(inputPosition).xyz;
    float lightDistanceSq = length2(fragInstancePosition - fragPosition);
    if (lightDistanceSq > fragInstanceRadiusSq) return;

    float attenuation = 1.0 / (1.0 + (251.0 / (5 * fragInstanceRadiusSq)) * lightDistanceSq);
    vec3 lightColor = fragInstanceColor * attenuation;
    vec3 fragNormal = 2.0 * subpassLoad(inputNormal).xyz - 1.0;

    // Diffuse.
    vec3 lightDir = normalize(fragInstancePosition - fragPosition);
    float diff = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular.
    vec3 viewDir = normalize(pc.viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(fragNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    outColor = vec4((diffuse + specular) * materialColor, 1.0);
}