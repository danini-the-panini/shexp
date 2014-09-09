#version 440
#define PI (3.1415926535897932384626433832795)

layout (location = 0) out float ip;

const int N_BANDS = 4;
const int N_COEFFS = N_BANDS*N_BANDS;

uniform sampler2D normal;
uniform sampler2DArray shlogs;

uniform int screen_width;
uniform int screen_height;

uniform float max_zh_len;
uniform sampler2D a_lut, b_lut, len_lut;
uniform samplerCubeArray h_maps;

float[N_COEFFS] lh(vec3 v)
{
  float[N_COEFFS] result;

  for(int i = 0; i < N_COEFFS; i++)
  {
    result[i] = texture(h_maps, vec4(v,i)).r;
  }

  return result;
}

float shdot(float[N_COEFFS] a, float[N_COEFFS] b)
{
  float sum = 0;
  for (int i = 0; i < N_COEFFS; i++)
  {
    sum += a[i] * b[i];
  }
  return sum;
}

float shlen(float[N_COEFFS] x)
{
  float l = 0;
  for (int i = 0; i < N_COEFFS; i++)
  {
    l += x[i]*x[i];
  }
  return sqrt(l);
}

float[N_COEFFS] shexp(float[N_COEFFS] f)
{
  float f_len = shlen(f);

  float[N_COEFFS] g;

  float e = exp(f[0]/sqrt(4.0/PI));

  float u = texture(len_lut, vec2(0.5,clamp(f_len/max_zh_len,0,1))).r;
  float a = texture(a_lut, vec2(0.5, u)).r;
  float b = texture(b_lut, vec2(0.5, u)).r;

  g[0] = a*sqrt(4.0*PI)*e;
  for (int i = 1; i < N_COEFFS; i++)
  {
    g[i] = b*f[i]*e;
  }

  return g;
}

float[N_COEFFS] get_sh_vector(sampler2DArray smplr, vec2 uv)
{
  float[N_COEFFS] result;

  for (int i = 0; i < N_COEFFS/4; i++)
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
  vec2 screen = vec2( gl_FragCoord.x / float(screen_width),
      gl_FragCoord.y / float(screen_height));

  vec3 p_normal = texture(normal, screen).xyz;
  float[N_COEFFS] f = get_sh_vector(shlogs, screen);

  ip = shdot(lh(p_normal), shexp(f));
}
