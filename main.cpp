#include <cstdlib>
#include <fstream>
#include <iostream>

#include "camera.h"
#include "cube_map.h"
#include "cube_map_array.h"
#include "framebuffer.h"
#include "gfx_boilerplate.h"
#include "green.h"
#include "ndc_quad.h"
#include "plane.h"
#include "sh_functions.h"
#include "shader.h"
#include "sphere.h"
#include "texture1d.h"
#include "texture1d_array.h"
#include "texture2d.h"
#include "transform.h"

using namespace std;

typedef OrthoRotMatCamera<float, highp> ORMCamF;
ORMCamF camera(ORMCamF::vec3_type(0,30,-50));

typedef function<double(double,double)> spherical_function;
typedef function<void(double,double,double*)> spherical_sh_function;

const int KEYS[] = {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
  GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_E, GLFW_KEY_Q};
void handleInput(GLFWwindow *window)
{
  for (unsigned i = 0; i < sizeof(KEYS)/sizeof(int); i++)
  {
    if (glfwGetKey(window, KEYS[i]) == GLFW_PRESS)
    {
      camera.doKey(KEYS[i]);
    }
  }
}

bool first_mouse = true;
double mx, my;
void mouse_callback(GLFWwindow *window, double x, double y)
{
  if (first_mouse)
  {
    first_mouse = false;
  } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
  {
    double dx = x - mx;
    double dy = y - my;

    camera.mouseLook((float)dx, (float)dy);
  }

  mx = x;
  my = y;
}

bool paused = false;
void my_key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
  key_callback(win, key, scancode, action, mods);

  if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    paused = !paused;
}

double safe_acos(double x)
{
  if (x < -1) return M_PI;
  if (x > 1) return 0;
  return acos(x);
}

template < typename T >
T dot_polar(T theta1, T phi1, T theta2, T phi2)
{
  return sin(theta1) * sin(theta2) * cos(phi1-phi2) + cos(theta1) * cos(theta2);
}

template < typename T >
T angle_between(T theta1, T phi1, T theta2, T phi2)
{
  T cospsi = dot_polar(theta1, phi1, theta2, phi2);
  return safe_acos(cospsi);
}

spherical_function overcast()
{
  return [&](double theta, double phi) {
    return (1.0 + 2.0*sin(M_PI*0.5 - theta))/3.0;
  };
}

double clamp(double x, double a, double b)
{
  return min(max(x,a),b);
}

spherical_function clearsky(const double sun_theta, const double sun_phi, const double scale = 0.2)
{
  const double Z = M_PI/2.0 - sun_theta;
  const double cos_Z = cos(Z);
  return [Z, cos_Z, sun_theta, sun_phi, scale](double theta, double phi) {
    double gamma = angle_between(sun_theta, sun_phi, theta, phi);
    double cos_gamma = cos(gamma);
    double num = (0.91f + 10 * exp(-3 * gamma) + 0.45 * cos_gamma * cos_gamma) * (1 - exp(-0.32f / cos(theta)));
    double denom = (0.91f + 10 * exp(-3 * Z) + 0.45 * cos_Z * cos_Z) * (1 - exp(-0.32f));
    return max(scale * num / denom, 0.0);
  };
}

spherical_function splodge(const double sun_theta, const double sun_phi)
{
  return [sun_theta, sun_phi](double theta, double phi) {
    return dot_polar(sun_theta, sun_phi, theta, phi);
  };
}

spherical_function allwhite()
{
  return [](double, double) {
    return 1;
  };
}

spherical_function testing_ground()
{
  return [&](double, double phi) {
    return phi/(2.0*M_PI);
  };
}

spherical_function h_function(const double n_theta, const double n_phi)
{
  return [n_theta, n_phi](double s_theta, double s_phi) {
    return max(dot_polar(s_theta, s_phi, n_theta, n_phi), 0.0);
  };
}

void convert_to_polar(double x, double y, double z, double *r, double *theta, double *phi)
{
  *r = sqrt(x*x+y*y+z*z);
  *theta = safe_acos(y / (*r));
  *phi = M_PI + atan2(z,x);
}

void fill_cube_map(float ** data, const GLsizei size,
    spherical_function fn)
{
  const float GU = size*0.5f;
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
    {
      float u = (float)j - GU + 0.5f;
      float v = (float)i - GU + 0.5f;

      vec3 d[6] = {
        vec3( GU,  -v,  -u),
        vec3(-GU,  -v,   u),
        vec3(  u,  GU,   v),
        vec3(  u, -GU,  -v),
        vec3(  u,  -v,  GU),
        vec3( -u,  -v, -GU)
      };

      for (int k = 0; k < 6; k++)
      {
        double x = d[k].x, y = d[k].y, z = d[k].z;

        double n_radius, n_theta, n_phi;

        convert_to_polar(x,y,z,&n_radius,&n_theta,&n_phi);

        data[k][i*size+j] = (float)fn(n_theta, n_phi);
      }
    }
  }
}

void fill_sh_cube_map_array(float * data, const GLsizei size, spherical_sh_function fn)
{
  double *coeffs = new double[N_COEFFS];
  float ***data_ptr = new float**[N_COEFFS];
  for (int i = 0; i < N_COEFFS; i++)
  {
    data_ptr[i] = new float*[6];
    for (int j = 0; j < 6; j++)
    {
      data_ptr[i][j] = data+i*(6*size*size)+j*(size*size);
    }
  }
  float GU = size*0.5f;
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
    {
      float u = (float)j - GU + 0.5f;
      float v = (float)i - GU + 0.5f;

      vec3 d[6] = {
        vec3( GU,  -v,  -u),
        vec3(-GU,  -v,   u),
        vec3(  u,  GU,   v),
        vec3(  u, -GU,  -v),
        vec3(  u,  -v,  GU),
        vec3( -u,  -v, -GU)
      };

      for (int k = 0; k < 6; k++)
      {
        double x = d[k].x, y = d[k].y, z = d[k].z;

        double n_radius, n_theta, n_phi;

        convert_to_polar(x,y,z,&n_radius,&n_theta,&n_phi);

        fn(n_theta, n_phi, coeffs);

        for(int index=0; index < N_COEFFS; ++index) {
          data_ptr[index][k][i*size+j] = coeffs[index];
        }
      }
    }
  }
  for (int i = 0; i < N_COEFFS; i++)
  {
    delete [] data_ptr[i];
  }
  delete [] data_ptr;
  delete [] coeffs;
}

CubeMap gen_cube_map(const GLsizei size, spherical_function fn,
    GLint internalFormat, GLenum format, GLenum type)
{
  float ** data = new float*[6];
  for (int i = 0; i < 6; i++)
    data[i] = new float[size*size];

  fill_cube_map(data, size, fn);

  CubeMap map;
  map.build();
  map.load_cube(data, size, internalFormat, format, type);

  for (int i = 0; i < 6; i++)
    delete [] data[i];
  delete [] data;

  return map;
}

CubeMapArray gen_sh_cube_map_array(const GLsizei size, spherical_sh_function fn,
    GLint internalFormat, GLenum format, GLenum type)
{
  float *data = new float[N_COEFFS*6*size*size];

  fill_sh_cube_map_array(data, size, fn);

  CubeMapArray map;
  map.build();
  map.load_cube_array(data, size, N_COEFFS,
    internalFormat, format, type);

  delete [] data;

  return map;
}

float sh_length(const float *x)
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

float lerp(float a, float b, float u)
{
  return a*(1-u) + b*u;
}

const int MAX_SPHERES = 256;

void update_sphere(int i, vec3 position, float radius,
    float *positions, float *radiuses)
{
  radiuses[i] = radius;
  positions[i] = position.x;
  positions[i+MAX_SPHERES] = position.y;
  positions[i+2*MAX_SPHERES] = position.z;
}

vec3 sphere_position(int i, float *positions)
{
  return vec3(
        positions[i],
        positions[i+MAX_SPHERES],
        positions[i+2*MAX_SPHERES]
      );
}

int main(int argc, char** argv)
{
  GFXBoilerplate gfx;
  gfx.init();
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  ////////////// LOAD SH LUT //////////////

  Texture2d sh_lut, a_lut, b_lut, len_lut;

  ifstream in(argc > 1 ? argv[0] : "sh_lut.txt");
  if (!in)
  {
    cout << "Error loading file " << (argc > 1 ? argv[0] : "sh_lut.txt") << endl;
    return 1;
  }
  int lut_size;
  in >> lut_size;
  float* sh_logs = new float[lut_size*N_COEFFS];
  float* a_coeffs = new float[lut_size];
  float* b_coeffs = new float[lut_size];
  float* inv_lens = new float[lut_size];

  double tmp;
  for (int i = 0; i < lut_size*N_COEFFS; i++)
  {
    in >> tmp;
    sh_logs[i] = static_cast<float>(tmp);
  }
  for (int i = 0; i < lut_size; i++)
  {
    in >> tmp;
    a_coeffs[i] = static_cast<float>(tmp);
  }
  for (int i = 0; i < lut_size; i++)
  {
    in >> tmp;
    b_coeffs[i] = static_cast<float>(tmp);
  }

  const float MAX_ZH_LENGTH = 8.845128074703045f;

  float lengths[lut_size];
  for (int i = 0; i < lut_size; i++)
  {
    lengths[i] = sh_length(sh_logs+i*N_COEFFS)/MAX_ZH_LENGTH;
  }

  for (int i = 0; i < lut_size; i++)
  {
    float fx = (float)i/(float)lut_size;
    int j = 1;
    while (lengths[j] < fx && j < lut_size) j++;

    float u = unlerp(lengths[j-1], lengths[j], fx);

    float x = (float)j - 1.f + u;
    inv_lens[i] = x/(float)lut_size;
  }

  GLuint tex_offset = 0;

  glActiveTexture(GL_TEXTURE0+(tex_offset++));
  sh_lut.build();
  sh_lut.load_tex(sh_logs, N_COEFFS, lut_size, GL_R32F, GL_RED, GL_FLOAT);

  glActiveTexture(GL_TEXTURE0+(tex_offset++));
  a_lut.build();
  a_lut.load_tex(a_coeffs, 1, lut_size, GL_R32F, GL_RED, GL_FLOAT);

  glActiveTexture(GL_TEXTURE0+(tex_offset++));
  b_lut.build();
  b_lut.load_tex(b_coeffs, 1, lut_size, GL_R32F, GL_RED, GL_FLOAT);

  glActiveTexture(GL_TEXTURE0+(tex_offset++));
  len_lut.build();
  len_lut.load_tex(inv_lens, 1, lut_size, GL_R32F, GL_RED, GL_FLOAT);

  delete [] sh_logs;
  delete [] a_coeffs;
  delete [] b_coeffs;
  delete [] inv_lens;

  ////////////// GENERATE LH CUBEMAPS ////////////

  const int SQRT_N_SAMPLES = 100;
  const int N_SAMPLES = SQRT_N_SAMPLES*SQRT_N_SAMPLES;
  SHSample* samples = new SHSample[N_SAMPLES];
  for (int i = 0; i < N_SAMPLES; i++)
  {
    samples[i].coeff = new double[N_COEFFS];
  }
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);

  double *l_coeff = new double[N_COEFFS];
  double *h_coeff = new double[N_COEFFS];

  auto sky_function = clearsky(M_PI*0.3, M_PI);

  SH_project_polar_function(sky_function, samples, N_SAMPLES, N_BANDS, l_coeff);

  const int H_MAP_SIZE = 8;

  auto h_map_function = [l_coeff, h_coeff, samples](double theta, double phi, double *coeffs)
  {
    SH_project_polar_function(h_function(theta, phi), samples, N_SAMPLES, N_BANDS, h_coeff);

    for (int h = 0; h < N_COEFFS; h++)
    {
      h_coeff[h] /= M_PI;
    }

    SH_product(h_coeff, l_coeff, coeffs);
  };

  cout << "Loading H map ... " << endl;
  glActiveTexture(GL_TEXTURE0+(tex_offset++));
  CubeMapArray h_map_array = gen_sh_cube_map_array(H_MAP_SIZE, h_map_function,
      GL_R32F, GL_RED, GL_FLOAT);
  cout << "done." << endl;

  delete [] l_coeff;
  delete [] h_coeff;


  ///////////////// MAKE SPHERE TEXTURE ////////////////

  int radiuses_slot = tex_offset++;
  int positions_slot = tex_offset++;

  float *radiuses_data = new float[MAX_SPHERES];
  float *positions_data = new float[MAX_SPHERES*3];

  glActiveTexture(GL_TEXTURE0+radiuses_slot);
  Texture1d radiuses;
  radiuses.build();
  radiuses.load_tex(radiuses_data, MAX_SPHERES,
      GL_R32F, GL_RED, GL_FLOAT);

  glActiveTexture(GL_TEXTURE0+positions_slot);
  Texture1dArray positions;
  positions.build();
  positions.load_tex(positions_data, MAX_SPHERES, 3,
      GL_R32F, GL_RED, GL_FLOAT);

  ///////////////// DO THE OPENGL THING ////////////////

  glfwSetCursorPosCallback(gfx.window(), mouse_callback);
  glfwSetKeyCallback(gfx.window(), my_key_callback);

  Shader* pos_shader = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("pos_frag.glsl")
    -> build();

  Shader* norm_shader = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("norm_frag.glsl")
    ->build();

  Shader* shexp_shader = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("sh_frag.glsl")
    ->build();

  Shader* main_shader = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("main_frag.glsl")
    ->build();

  Shader* skybox = (new Shader())
    ->vertex("skybox_vert.glsl")
    ->fragment("skybox_frag.glsl")
    ->build();

  Framebuffer pos_buffer(256,256);
  pos_buffer.build();
  Framebuffer norm_buffer(256,256);
  norm_buffer.build();
  Framebuffer shexp_buffer(256,256);
  shexp_buffer.build();

  Sphere sph;
  sph.build();

  Plane pln;
  pln.build();

  glActiveTexture(GL_TEXTURE0+42);
  CubeMap skymap = gen_cube_map(256, sky_function, GL_R32F, GL_RED, GL_FLOAT);

  NDCQuad sky;
  sky.build();

  const GLuint levels = 4;
  const GLuint segments = 16;
  int num_spheres = 1+(segments*levels);
  vector<vec3> colors;

  update_sphere(0, vec3(0,24,0), 24.f, positions_data, radiuses_data);
  colors.push_back(vec3(0,1,1));

  const GLfloat rad_per_lng = (2.f*(GLfloat)M_PI) / (GLfloat)segments;
  for (int j = 0; j < levels; j++)
  {
    for (int i = 0; i < segments; i++)
    {
      update_sphere(1+j*segments+i, vec3(
              50.0 * cos(i * rad_per_lng),
              6.0+(12.0*j),
              50.0 * sin(i * rad_per_lng)
            ), 6.f,
          positions_data, radiuses_data);
      colors.push_back(vec3(1,0,1));
    }
  }

  vector<Transform> transforms;
  for (int i = 0; i < num_spheres; i++)
  {
    transforms.push_back(Transform(sphere_position(i,positions_data), quat(1,0,0,0),
          vec3(radiuses_data[i])));
  }

  Transform plane_transform(vec3(0,0,0),quat(1,0,0,0),vec3(200));

  float x = 0.0f;
  float far = 1000.0f;
  int width, height;
  float aspect;
  mat4 projection;

  shexp_shader->use();
  shexp_shader->updateInt("sh_lut", 0);
  shexp_shader->updateInt("a_lut", 1);
  shexp_shader->updateInt("b_lut", 2);
  shexp_shader->updateInt("len_lut", 3);
  shexp_shader->updateInt("h_maps", 4);
  shexp_shader->updateInt("radiuses", 5);
  shexp_shader->updateInt("positions", 6);
  shexp_shader->updateInt("n_spheres", num_spheres);
  shexp_shader->updateFloat("max_zh_len", MAX_ZH_LENGTH);

  skybox->use();
  skybox->updateInt("map", 42);

  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(gfx.window()))
  {
    glfwGetFramebufferSize(gfx.window(), &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    aspect = (float)width/(float)height;
    projection = perspective(45.0f, aspect, 0.5f, far);

    if (!paused)
      x += 0.01f;

    handleInput(gfx.window());

    skybox->use();
    skybox->updateFloat("aspect", aspect);
    skybox->updateMat4("view", camera.getView());
    skybox->updateMat4("projection", projection);

    glDepthMask(GL_FALSE);
    sky.draw();
    glDepthMask(GL_TRUE);

    norm_shader->use();
    norm_shader->updateMat4("view", camera.getView());
    norm_shader->updateMat4("projection", projection);

    vec3 v = vec3(0,34+10*sin(x), 0);
    update_sphere(0, v, 24.f, positions_data, radiuses_data);
    transforms[0].set_translation(v);

    for (int j = 0; j < levels; j++)
    {
      GLfloat sx = (GLfloat)sin(x*2+(M_PI*0.25f*j));
      for (int i = 0; i < segments; i++)
      {
        v = vec3(
            (50.0+20*sx) * cos(i * rad_per_lng + x*(0.1*j)),
            6.0+(12.0*j),
            (50.0+20*sx) * sin(i * rad_per_lng + x*(0.1*j))
          );
        update_sphere(1+j*segments+i, v, 6.f,
            positions_data, radiuses_data);
        transforms[i+1+segments*j].set_translation(v);
      }
    }

    glActiveTexture(GL_TEXTURE0+radiuses_slot);
    radiuses.load_tex(radiuses_data, MAX_SPHERES,
        GL_R32F, GL_RED, GL_FLOAT);
    glActiveTexture(GL_TEXTURE0+positions_slot);
    positions.load_tex(positions_data, MAX_SPHERES, 3,
        GL_R32F, GL_RED, GL_FLOAT);

    for (int i = 0; i < num_spheres; i++)
    {
      norm_shader->updateMat4("world", transforms[i].world());
      norm_shader->updateVec3("color", colors[i]);
      sph.draw();
    }

    norm_shader->updateMat4("world", plane_transform.world());
    norm_shader->updateVec3("color", vec3(1,1,1));
    pln.draw();

    glfwSwapBuffers(gfx.window());
    glfwPollEvents();
  }

  sph.destroy();
  pln.destroy();

  shexp_shader->destroy();
  skybox->destroy();
  pos_shader->destroy();
  norm_shader->destroy();
  main_shader->destroy();
  delete shexp_shader;
  delete skybox;
  delete pos_shader;
  delete norm_shader;
  delete main_shader;

  gfx.cleanup();

  return 0;
}
