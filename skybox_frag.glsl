#version 420

uniform samplerCube map;

in vec3 dir;

layout (location=0) out vec4 colour;

void main()
{
  colour = texture(map, normalize(dir));
}
