#version 330

layout (location = 0) out vec4 out_color;

uniform vec3 sharm[9];

uniform vec3 lights[2];
uniform vec3 ld[2];

in vec3 f_normal;
in vec3 f_position;
in vec3 eye;

vec3 lightNormal(vec3 dirn)
{
  vec3 colour = sharm[0];
  colour += sharm[1] * dirn.x;
  colour += sharm[2] * dirn.z;
  colour += sharm[3] * dirn.x;
  colour += sharm[4] * (dirn.x * dirn.y);
  colour += sharm[5] * (dirn.y * dirn.z);
  colour += sharm[6] * (3.0f * dirn.z * dirn.z - 1.0f);
  colour += sharm[7] * (dirn.z * dirn.x);
  colour += sharm[8] * (dirn.x * dirn.x - dirn.y * dirn.y);
  return colour;
}

void main()
{
    vec3 ambient = vec3(0,0,0);

    float ia = 0.0f;
    float id = 0.7f;
    float is = 0.3f;
    float s = 100.0f;
    
    vec3 v = normalize(eye-f_position);

    for (int i = 0; i < 2; i++)
    {
      vec3 l = normalize(lights[i]);
      vec3 r = normalize(reflect(-l,f_normal));

      float ip = ia + max(dot(l,f_normal),0)*id + pow(max(dot(r,v),0),s)*is;
      ambient += ip*ld[i];
    }

    ambient += lightNormal(f_normal);

    out_color = vec4(ambient,1);
}

