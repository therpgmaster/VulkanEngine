#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWS;
layout(location = 2) in vec3 fragNormalWS;

layout (location = 0) out vec4 outColor;

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
	gl_FragDepth = 0.7;
	//outColor = vec4(fragPositionWS, 1.0);
	outColor = vec4(0.006, 0.006, 0.006, 0.75);
}