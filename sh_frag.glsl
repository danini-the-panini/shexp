#version 440
#define PI (3.1415926535897932384626433832795)

layout (location = 0) out vec4 out_color;

const int N = 1+(16*4);
const int N_BANDS = 4;
const int N_COEFFS = N_BANDS*N_BANDS;

in vec3 v_position;
in vec3 v_normal;

uniform vec3 color;
uniform float radiuses[N];
uniform vec3 positions[N];

uniform sampler2D sh_lut;
uniform samplerCube h_maps[N_COEFFS];

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
    result[i] = texture(h_maps[i], v).r;
  }

  return result;
}

// assume v is normalized!
float[N_COEFFS] y(vec3 v)
{
  return float[N_COEFFS](
    0.5*sqrt(1/PI),

    sqrt(3/4*PI) * v.y,
    sqrt(3/4*PI) * v.z,
    sqrt(3/4*PI) * v.x,

    0.5*sqrt(15/PI)*v.x*v.y,
    0.5*sqrt(15/PI)*v.y*v.z,
    0.25*sqrt(5/PI)*(-(v.x*v.x)-(v.y*v.y)+2*(v.z*v.z)),
    0.5*sqrt(15/PI)*v.z*v.x,
    0.25*sqrt(5/PI)*((v.x*v.x)-(v.y*v.y)),

    0.25*sqrt(35/2*PI)*(3*v.x*v.x - v.y*v.y)*v.y,
    0.5*sqrt(105/PI)*v.x*v.y*v.z,
    0.25*sqrt(21/PI)*v.y*(4*v.z*v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(7/PI)*v.z*(2*v.z*v.z-3*v.x*v.x-3*v.y*v.y),
    0.25*sqrt(21/2*PI)*v.x*(4*v.z-v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(105/PI)*(v.x*v.x-v.y*v.y)*v.z,
    0.25*sqrt(35/2*PI)*(v.x*v.x-3*v.y*v.y)*v.x

    // ...
  );
}

float[N_COEFFS] rotate_to(float[N_COEFFS] sh, vec3 v)
{
  float[N_COEFFS] yv = y(v);
  float sh0 = sh[0]/sqrt(4*PI);
  float sh1 = sh[2]/sqrt((4*PI)/3);
  float sh2 = sh[6]/sqrt((4*PI)/3);
  float sh3 = sh[12]/sqrt((4*PI)/3);
  // ...

  return float[N_COEFFS](
    sh0*yv[0],
    sh1*yv[1],sh1*yv[2],sh1*yv[3],
    sh2*yv[4],sh2*yv[5],sh2*yv[6],sh2*yv[7],sh2*yv[8],
    sh3*yv[9],sh3*yv[10],sh3*yv[11],sh3*yv[12],sh3*yv[13],sh3*yv[14],sh3*yv[15]
    // ...
  );
}

float dot_sh(float[N_COEFFS] a, float[N_COEFFS] b)
{
  float sum = 0;
  for (int i = 0; i < N_COEFFS; i++)
  {
    sum += a[i] * b[i];
  }
  return sum;
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
    log_coeff[j] = texture(sh_lut, vec2(float(j)/float(N_COEFFS),fi)).r;
  }
  return rotate_to(log_coeff, normalize(v));
}

void main()
{
  float[N_COEFFS] acc_coeff;
  for (int i = 0; i < N_COEFFS; i ++)
  {
    acc_coeff[i] = 0;
  }
  for (int i = 0; i < N; i++)
  {
    vec3 v = positions[i] - v_position;
    float[N_COEFFS] rlog_coeff = get_coeff(v, radiuses[i]);
    for (int j = 0; j < N_COEFFS; j++)
    {
      acc_coeff[j] += rlog_coeff[j];
    }
  }
  acc_coeff[0] += sqrt(4*PI);

  float ip = dot_sh(y(v_normal), acc_coeff);

  out_color = vec4(color * pow(ip,2), 1);

  /*out_color = vec4(texture(h_maps[0], v_normal).r, texture(h_maps[1], v_normal).r, texture(h_maps[2], v_normal).r, 1);*/
}

