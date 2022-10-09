#version 450
#extension GL_EXT_scalar_block_layout: require
// inputs from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWS;
layout(location = 2) in vec3 fragNormalWS;
layout(location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;
layout (depth_any) out float gl_FragDepth;

struct testStruct
{
	float s;
	vec3 v;
};

layout(std430, set = 0, binding = 0) uniform UBO1 
{
	mat4 projectionViewMatrix;
	testStruct[2] test;
} ubo1;

layout(set = 0, binding = 1) uniform texture2D textures[2];
layout(set = 0, binding = 2) uniform sampler _sampler;

layout(push_constant) uniform Push
{
	mat4 transform;
	mat4 normalMatrix;
} push;

void main() 
{
	gl_FragDepth = 0.9;
	//outColor = texture(texSampler, fragUV);
	outColor = texture(sampler2D(textures[1], _sampler), fragUV);
}