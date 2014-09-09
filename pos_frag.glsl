#version 440

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;

in vec3 v_position;
in vec3 v_normal;

void main()
{
  position = v_position;
  normal = v_normal;
}
