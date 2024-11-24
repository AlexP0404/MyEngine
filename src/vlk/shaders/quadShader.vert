#version 450

layout(binding = 0) uniform UniformBufferObject {
  vec4 movedPos[400];
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in uint quadID;

layout(location = 0) out vec3 fragColor;


void main(){
  gl_Position = vec4(inPosition,0,1.0);
  gl_Position.xy += ubo.movedPos[quadID].xy;
  fragColor = inColor; 
}
