#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push	
{
	mat4 transform;
	vec3 color;
} push;

void main() {
  gl_Position = push.transform * position;

  // color negative normals gray
  vec3 nc = normal;
  if (nc.x < 0.0001 && nc.y < 0.0001 && nc.z < 0.0001) { nc = vec3(0.06); }
  fragColor = nc; 
}