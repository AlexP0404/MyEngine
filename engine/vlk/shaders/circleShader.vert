#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec3 worldPosition[400];
    mat4 viewMat;
} ubo;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in float inRadius;
layout(location = 2) in vec3 inColor;
layout(location = 3) in uint quadID;
layout(location = 4) in vec2 inLocalPosition;

struct VertexOutput
{
    vec2 LocalPosition;
    vec3 Color;
    float Radius;
};

layout(location = 0) out VertexOutput Output;

void main() {
    gl_Position = ubo.viewMat * vec4(inWorldPosition + ubo.worldPosition[quadID], 1.0);
    Output.LocalPosition = inLocalPosition;
    Output.Color = inColor;
    Output.Radius = inRadius;
}
