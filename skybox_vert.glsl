#version 420
layout (location=1) in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform float aspect;

out vec3 dir;

void main()
{
  mat4 r = view;
  r[3][0] = 0.0;
  r[3][1] = 0.0;
  r[3][2] = 0.0;

  vec4 v = inverse(r) * inverse(projection) * vec4(position,1.0f);

  dir = v.xyz;

  gl_Position = vec4(position,1.0f);
}
