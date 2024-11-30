#version 450

layout(binding = 0) uniform UniformBufferObject {
  vec4 movedPos[400];
} ubo;

layout(location = 0) in vec2 inWorldPosition;
layout(location = 1) in vec2 inLocalPosition;
layout(location = 2) in vec3 inColor;
layout(location = 3) in float inRadius;
layout(location = 4) in uint quadID;

struct VertexOutput
{
  vec2 LocalPosition;
  vec3 Color;
  float Radius;
};

layout(location = 0) out VertexOutput Output;


void main(){
  gl_Position = vec4(inWorldPosition,0,1.0);
  gl_Position.xy += ubo.movedPos[quadID].xy;
  Output.LocalPosition = inLocalPosition;
  Output.Color = inColor;
  Output.Radius = inRadius;
}
