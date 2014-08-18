#version 440
#define PI (3.1415926535897932384626433832795)

layout (location = 0) out vec4 out_color;

const int N = 17;
const int N_COEFF = 9;
const int LUT_SIZE = 10;

in vec3 v_position;
in vec3 v_normal;

uniform vec3 color;
uniform float radiuses[N];
uniform vec3 positions[N];
uniform float sh_lut[N_COEFF*LUT_SIZE];

layout (location=0) uniform samplerCube h_maps0;
layout (location=1) uniform samplerCube h_maps1;
layout (location=2) uniform samplerCube h_maps2;
layout (location=3) uniform samplerCube h_maps3;
layout (location=4) uniform samplerCube h_maps4;
layout (location=5) uniform samplerCube h_maps5;
layout (location=6) uniform samplerCube h_maps6;
layout (location=7) uniform samplerCube h_maps7;
layout (location=8) uniform samplerCube h_maps8;
// ...

float[N_COEFF] lh(vec3 v)
{
  float[N_COEFF] result;

  result[0] = texture(h_maps0, v).r;
  result[1] = texture(h_maps1, v).r;
  result[2] = texture(h_maps2, v).r;
  result[3] = texture(h_maps3, v).r;
  result[4] = texture(h_maps4, v).r;
  result[5] = texture(h_maps5, v).r;
  result[6] = texture(h_maps6, v).r;
  result[7] = texture(h_maps7, v).r;
  result[8] = texture(h_maps8, v).r;
  // ...

  return result;
}

// assume v is normalized!
float[N_COEFF] y(vec3 v)
{
  return float[N_COEFF](
    0.5*sqrt(1/PI),

    sqrt(3/4*PI) * v.y,
    sqrt(3/4*PI) * v.z,
    sqrt(3/4*PI) * v.x,

    0.5*sqrt(15/PI)*v.x*v.y,
    0.5*sqrt(15/PI)*v.y*v.z,
    0.25*sqrt(5/PI)*(-(v.x*v.x)-(v.y*v.y)+2*(v.z*v.z)),
    0.5*sqrt(15/PI)*v.z*v.x,
    0.25*sqrt(5/PI)*((v.x*v.x)-(v.y*v.y))

    // ...
  );
}

float[N_COEFF] rotate_to(float[N_COEFF] sh, vec3 v)
{
  float[N_COEFF] yv = y(v);
  float sh0 = sh[0]/sqrt(4*PI);
  float sh1 = sh[2]/sqrt((4*PI)/3);
  float sh2 = sh[6]/sqrt((4*PI)/3);
  // ...

  return float[N_COEFF](
    sh0*yv[0],
    sh1*yv[1],sh1*yv[2],sh1*yv[3],
    sh2*yv[4],sh2*yv[5],sh2*yv[6],sh2*yv[7],sh2*yv[8]
    // ...
  );
}

float dot_sh(float[N_COEFF] a, float[N_COEFF] b)
{
  float sum = 0;
  for (int i = 0; i < N_COEFF; i++)
  {
    sum += a[i] * b[i];
  }
  return sum;
}

float angular_radius(float d, float r)
{
  return asin(r/d);
}

float[N_COEFF] get_coeff(vec3 v, float radius)
{
  float d = length(v);
  float ar = angular_radius(d, radius);
  float fi = clamp(ar/(PI*0.5),0,1)*LUT_SIZE;
  int ii = int(fi);
  float btw = fi-float(ii);
  int lut_index = ii*N_COEFF;
  float[N_COEFF] log_coeff;
  for (int j = 0; j < N_COEFF; j++)
  {
    log_coeff[j] = mix(sh_lut[j+lut_index], sh_lut[j+lut_index+N_COEFF], btw);
  }
  return rotate_to(log_coeff, normalize(v));
}

void main()
{
  float[N_COEFF] acc_coeff;
  for (int i = 0; i < N_COEFF; i ++)
  {
    acc_coeff[i] = 0;
  }
  for (int i = 0; i < N; i++)
  {
    vec3 v = positions[i] - v_position;
    float[N_COEFF] rlog_coeff = get_coeff(v, radiuses[i]);
    for (int j = 0; j < N_COEFF; j++)
    {
      acc_coeff[j] += rlog_coeff[j];
    }
  }
  acc_coeff[0] += sqrt(4*PI);

  float[N_COEFF] y_norm = lh(v_normal);
  float ip = dot_sh(acc_coeff, y_norm);

  /*out_color = vec4(color * pow(ip,10), 1);*/

  out_color = vec4(texture(h_maps0, v_normal).r, texture(h_maps1, v_normal).r, texture(h_maps2, v_normal).r, 1);
}

