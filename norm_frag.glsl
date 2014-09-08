#version 440

layout (location = 0) out vec4 out_colour;

in vec3 v_position;
in vec4 s_normal;
in vec3 v_eye;
in vec3 v_eye_norm;

void main()
{
  float zp = 1.0; //dot(v_position - v_eye, v_eye_norm);
  vec3 n = normalize(s_normal.xyz);
  out_colour = vec4((n+vec3(1))/2, zp);
}
