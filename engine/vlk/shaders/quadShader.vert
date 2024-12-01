#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec3 worldPosition[400];
    mat4 viewMat;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint quadID;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.viewMat * vec4(inPosition + ubo.worldPosition[quadID], 1.0);
    fragColor = inColor;
}
