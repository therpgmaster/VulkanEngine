#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPositionWS;
layout(location = 2) out vec3 fragNormalWS;

layout(set = 0, binding = 0) uniform GlobalFrameDataBuffer 
{
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} globalFrameData;

layout(push_constant) uniform Push	
{
	mat4 transform;
	mat4 normalMatrix;
} push;

void main() 
{
  gl_Position = globalFrameData.projectionViewMatrix * push.transform * position;
  fragNormalWS = normalize(mat3(push.normalMatrix) * normal);
  fragPositionWS = vec4(push.transform * position).xyz;
  fragColor = vec3(0.8, 0.6, 0.6); // use fixed value instead of vertex color
}