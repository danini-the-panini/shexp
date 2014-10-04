#version 440

layout (location = 0) out vec3 position;
layout (location = 1) out vec4 normal;

in vec3 v_position;
in vec3 v_normal;

in vec3 v_eye;
in vec3 v_eye_norm;

void main()
{
  position = v_position;
  float zp = dot(v_position - v_eye, v_eye_norm);
  normal = vec4(v_normal, zp);
}
