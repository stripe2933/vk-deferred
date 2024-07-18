#version 450

const vec3 rec709Luma = vec3(0.2126, 0.7152, 0.0722);

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputHdrColor;

layout (location = 0) out vec4 outColor;

void main(){
    vec3 hdrColor = subpassLoad(inputHdrColor).rgb;
    float luminance = dot(hdrColor, rec709Luma);
    outColor = vec4(hdrColor / (1.0 + luminance), 1.0);
}