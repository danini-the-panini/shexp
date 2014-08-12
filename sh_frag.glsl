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

uniform samplerCube h_maps[3];

float[N_COEFF] h(vec3 v)
{
  float[N_COEFF] result;

  vec4 a = texture(h_maps[0], v);
  result[0] = a.r;
  result[1] = a.g;
  result[2] = a.b;
  vec4 b = texture(h_maps[1], v);
  result[3] = b.r;
  result[4] = b.g;
  result[5] = b.b;
  vec4 c = texture(h_maps[2], v);
  result[6] = c.r;
  result[7] = c.g;
  result[8] = c.b;

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

  float[N_COEFF] y_norm = h(v_normal);
  float ip = dot_sh(acc_coeff, y_norm);

  out_color = vec4(color * pow(ip,10), 1);
}

