#version 440

uniform samplerCube map;

in vec3 dir;

layout (location = 0) out vec4 colour;

void main()
{
  colour = vec4(vec3(texture(map, normalize(dir)).r),1);
}
