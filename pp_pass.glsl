#version 440

uniform int screen_width;
uniform int screen_height;

uniform sampler2DArray tex1;
uniform sampler2D tex2;

layout (location = 0) out vec4 out_color;

float shlen(float[16] x)
{
  float l = 0;
  for (int i = 0; i < 16; i++)
  {
    l += x[i]*x[i];
  }
  return sqrt(l);
}

float[16] get_sh_vector(sampler2DArray smplr, vec2 uv)
{
  float[16] result;

  for (int i = 0; i < 4; i++)
  {
    result[i*4]   = texture(smplr, vec3(uv, i)).r;
    result[i*4+1] = texture(smplr, vec3(uv, i)).g;
    result[i*4+2] = texture(smplr, vec3(uv, i)).b;
    result[i*4+3] = texture(smplr, vec3(uv, i)).a;
  }

  return result;
}

void main()
{
  float u = gl_FragCoord.x / float(screen_width);
  float v = gl_FragCoord.y / float(screen_height);

  /*out_color = mix(texture(tex1, vec2(u,v)),*/
      /*texture(tex2, vec2(u,v)), 0.5);*/

  float shexp = texture(tex2, vec2(u,v)).r;
  float len = shlen(get_sh_vector(tex1, vec2(u,v)));

  out_color = vec4(shexp,shexp,shexp, 1);
}
