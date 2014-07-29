#version 330

layout (location = 0) out vec4 out_color;

const int N = 2;

in vec3 v_position;

uniform vec3 color;
uniform float radiuses[N];
uniform vec3 positions[N];

float angular_radius(float d, float r)
{
  return asin(r/d);
}

void main()
{
  for (int i = 0; i < N; i++)
  {
    float d = length(v_position - positions[i]);
    float ar = angular_radius(d, radiuses[i]);
  }
  out_color = vec4(color, 1);
}

