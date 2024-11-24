#version 450

struct VertexOutput
{
  vec2 LocalPosition;
  vec3 Color;
  float Radius;
};

layout(location = 0) in VertexOutput Input;

layout(location = 0) out vec4 outColor;

void main(){
  float distance = 1.0 - length(Input.LocalPosition);
  float circle = smoothstep(0,Input.Radius,distance);
  circle *= smoothstep(Input.Radius,0.001+Input.Radius, distance);
  //float circle = smoothstep(0,0.005,distance);
  if(circle == 0.0) discard;

  //outColor = vec4(Input.Color,1.0f);
  outColor = vec4(Input.Color,circle);
}
