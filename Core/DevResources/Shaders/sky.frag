#version 450
// inputs from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWS;
layout(location = 2) in vec3 fragNormalWS;
layout(location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;
layout (depth_any) out float gl_FragDepth;

layout(std140, set = 0, binding = 0) uniform UBO1 
{
	mat4 projectionViewMatrix;
} ubo1;

layout(std140, set = 0, binding = 1) uniform UBO2 
{
	vec3 hue;
} ubo2;

layout(set = 0, binding = 3) uniform sampler2D texSampler;

layout(push_constant) uniform Push	
{
	mat4 transform;
	mat4 normalMatrix;
} push;

void main() 
{
	gl_FragDepth = 0.9;
	outColor = texture(texSampler, fragUV);
}