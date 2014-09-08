#version 440
#define PI (3.1415926535897932384626433832795)

layout (location = 0) out vec4 out_color;

const int N_BANDS = 4;
const int N_COEFFS = N_BANDS*N_BANDS;

in vec3 v_position;
in vec3 v_normal;

uniform vec3 color;
uniform int n_spheres;
uniform sampler1D radiuses;
uniform sampler1DArray positions;

uniform float max_zh_len;
uniform sampler2D sh_lut, a_lut, b_lut, len_lut;
uniform samplerCubeArray h_maps;

vec3 sphere_position(int i)
{
  return vec3(
        texelFetch(positions, ivec2(i,0), 0).r,
        texelFetch(positions, ivec2(i,1), 0).r,
        texelFetch(positions, ivec2(i,2), 0).r
      );
}

float sphere_radius(int i)
{
  return texelFetch(radiuses, i, 0).r;
}

float[N_COEFFS] window(float[N_COEFFS] c)
{
  float h = 2.0*N_BANDS;
  float[N_COEFFS] result;
  for (int l = 0; l < N_BANDS; l++)
  {
    float a = cos(PI/2.0*(float(l)/h));
    for (int m = -l; m <= l; m++)
    {
      int i = l*(l+1)+m;
      result[i] = c[i] * a;
    }
  }
  return result;
}

float[N_COEFFS] lh(vec3 v)
{
  float[N_COEFFS] result;

  for(int i = 0; i < N_COEFFS; i++)
  {
    result[i] = texture(h_maps, vec4(v,float(i))).r;
  }

  return result;
}

// assume v is normalized!
float[N_COEFFS] y(vec3 vo)
{
  vec3 v = vo.xzy;
  return float[N_COEFFS](
    0.5*sqrt(1.0/PI),

    sqrt(3.0/(4.0*PI)) * v.y,
    sqrt(3.0/(4.0*PI)) * v.z,
    sqrt(3.0/(4.0*PI)) * v.x,

    0.5*sqrt(15.0/PI)*v.x*v.y,
    0.5*sqrt(15.0/PI)*v.y*v.z,
    0.25*sqrt(5.0/PI)*(-(v.x*v.x)-(v.y*v.y)+2.0*(v.z*v.z)),
    0.5*sqrt(15.0/PI)*v.z*v.x,
    0.25*sqrt(15.0/PI)*((v.x*v.x)-(v.y*v.y)),

    0.25*sqrt(35.0/(2.0*PI))*(3.0*v.x*v.x - v.y*v.y)*v.y,
    0.5*sqrt(105.0/PI)*v.x*v.y*v.z,
    0.25*sqrt(21.0/(2.0*PI))*v.y*(4.0*v.z*v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(7.0/PI)*v.z*(2.0*v.z*v.z-3.0*v.x*v.x-3.0*v.y*v.y),
    0.25*sqrt(21.0/(2.0*PI))*v.x*(4.0*v.z-v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(105.0/PI)*(v.x*v.x-v.y*v.y)*v.z,
    0.25*sqrt(35.0/(2.0*PI))*(v.x*v.x-3.0*v.y*v.y)*v.x

    // ...
  );
}

float[N_COEFFS] rotate_to(float[N_COEFFS] sh, vec3 v)
{
  float[N_COEFFS] yv = y(v);
  float sh0 = sh[0]/sqrt(4.0*PI);
  float sh1 = sh[2]/sqrt((4.0*PI)/3.0);
  float sh2 = sh[6]/sqrt((4.0*PI)/5.0);
  float sh3 = sh[12]/sqrt((4.0*PI)/7.0);
  // ...

  return float[N_COEFFS](
    sh0*yv[0],
    sh1*yv[1],sh1*yv[2],sh1*yv[3],
    sh2*yv[4],sh2*yv[5],sh2*yv[6],sh2*yv[7],sh2*yv[8],
    sh3*yv[9],sh3*yv[10],sh3*yv[11],sh3*yv[12],sh3*yv[13],sh3*yv[14],sh3*yv[15]
    // ...
  );
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

float unlerp(float a, float b, float x)
{
  return (x-a)/(b-a);
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

float angular_radius(float d, float r)
{
  return asin(r/d);
}

float[N_COEFFS] get_coeff(vec3 v, float radius)
{
  float d = length(v);
  float ar = angular_radius(d, radius);
  float fi = clamp(ar/(PI*0.5),0,1);
  float[N_COEFFS] log_coeff;
  for (int j = 0; j < N_COEFFS; j++)
  {
    log_coeff[j] = texture(sh_lut, vec2((float(j)+0.5)/float(N_COEFFS),fi)).r;
  }
  return rotate_to(log_coeff, normalize(v));
}

void main()
{
  float[N_COEFFS] f;
  for (int i = 0; i < N_COEFFS; i++)
  {
    f[i] = 0;
  }
  for (int i = 0; i < n_spheres; i++)
  {
    vec3 v = sphere_position(i) - v_position;
    float[N_COEFFS] fi = get_coeff(v, sphere_radius(i));
    for (int j = 0; j < N_COEFFS; j++)
    {
      f[j] += fi[j];
    }
  }
  float ip = shdot(lh(v_normal), shexp(f));

  out_color = vec4(color * ip, 1);
}

