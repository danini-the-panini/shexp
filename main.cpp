#include <cstdlib>
#include <iostream>
#include <fstream>

#include "gfx_boilerplate.h"
#include "sh_functions.h"
#include "shader.h"
#include "sphere.h"
#include "plane.h"
#include "ndc_quad.h"
#include "camera.h"
#include "transform.h"
#include "texture2d.h"
#include "cube_map.h"
#include "green.h"

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

void fill_sh_cube_maps(float *** data, const GLsizei size, spherical_sh_function fn)
{
  double *coeffs = new double[N_COEFFS];
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
          data[index][k][i*size+j] = coeffs[index];
        }
      }
    }
  }
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
  map.load_cube(data, size, size, internalFormat, format, type);

  for (int i = 0; i < 6; i++)
    delete [] data[i];
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

int main(int argc, char** argv)
{
  GFXBoilerplate gfx;
  gfx.init();
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  ////////////// LOAD SH LUT //////////////
  cout << "Loading SH LUT ... ";

  Texture2d sh_lut, a_lut, b_lut;

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

  cout << "--- SH LENGTHS ---" << endl;
  for (int i = 0; i < lut_size; i++)
  {
    cout << sh_length(sh_logs+i*N_COEFFS) << endl;
  }
  cout << "------------------" << endl;

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

  cout << "done." << endl;

  ////////////// GENERATE LH CUBEMAPS ////////////
  cout << "Loading LH CUBEMAPS ... " << endl;

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

  auto sky_function = clearsky(M_PI*0.2, M_PI);

  SH_project_polar_function(sky_function, samples, N_SAMPLES, N_BANDS, l_coeff);

  const int CUBE_MAP_SIZE = 8;

  CubeMap h_maps[N_COEFFS];
  float **h_data[N_COEFFS];
  for (int i = 0; i < N_COEFFS; i++)
  {
    h_data[i] = new float*[6];
    for (int j = 0; j < 6; j++)
    {
      h_data[i][j] = new float[CUBE_MAP_SIZE*CUBE_MAP_SIZE];
    }
  }

  cout << " * Generating data ... \033[s";
  fill_sh_cube_maps(h_data, CUBE_MAP_SIZE,
    [l_coeff, h_coeff, samples](double theta, double phi, double *coeffs)
    {
      SH_project_polar_function(h_function(theta, phi), samples, N_SAMPLES, N_BANDS, h_coeff);

      for (int h = 0; h < N_COEFFS; h++)
      {
        h_coeff[h] /= M_PI;
      }

      SH_product(l_coeff, h_coeff, coeffs);
    });
  cout << "done." << endl;

  cout << " * Loading maps into textures ... ";
  for (int i = 0; i < N_COEFFS; i++)
  {
    glActiveTexture(GL_TEXTURE0+tex_offset+i);
    h_maps[i].build();
    h_maps[i].load_cube(h_data[i], CUBE_MAP_SIZE, CUBE_MAP_SIZE,
        GL_R32F, GL_RED, GL_FLOAT);
  }
  cout << "done." << endl;

  delete [] l_coeff;
  delete [] h_coeff;
  for (int i = 0; i < N_COEFFS; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      delete [] h_data[i][j];
    }
    delete [] h_data[i];
  }

  cout << "done." << endl;

  ///////////////// DO THE OPENGL THING ////////////////

  glfwSetCursorPosCallback(gfx.window(), mouse_callback);
  glfwSetKeyCallback(gfx.window(), my_key_callback);

  Shader* pass = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("sh_frag.glsl")
    ->build();

  Shader* skybox = (new Shader())
    ->vertex("skybox_vert.glsl")
    ->fragment("skybox_frag.glsl")
    ->build();

  Sphere sph;
  sph.build();

  Plane pln;
  pln.build();

  glActiveTexture(GL_TEXTURE0+42);
  CubeMap skymap = gen_cube_map(256, sky_function, GL_R32F, GL_RED, GL_FLOAT);

  NDCQuad sky;
  sky.build();

  pass->use();
  int indices[N_COEFFS];
  for (int i = 0; i < N_COEFFS; i++)
  {
    indices[i] = tex_offset+i;
  }
  pass->updateInt("sh_lut", 0);
  pass->updateInt("a_lut", 1);
  pass->updateInt("b_lut", 2);
  pass->updateInts("h_maps", indices, N_COEFFS);

  skybox->use();
  skybox->updateInt("map", 42);

  vector<vec3> sphere_positions;
  vector<GLfloat> sphere_radiuses;
  vector<vec3> colors;

  sphere_positions.push_back(vec3(0,24,0));
  sphere_radiuses.push_back(24.0f);
  colors.push_back(vec3(0,1,1));

  const GLuint levels = 4;
  const GLuint segments = 16;
  const GLfloat rad_per_lng = (2.f*(GLfloat)M_PI) / (GLfloat)segments;
  for (int j = 0; j < levels; j++)
  {
    for (int i = 0; i < segments; i++)
    {
      sphere_positions.push_back(vec3(
            50.0 * cos(i * rad_per_lng),
            6.0+(12.0*j),
            50.0 * sin(i * rad_per_lng)
            ));
      sphere_radiuses.push_back(6.0f);
      colors.push_back(vec3(1,0,1));
    }
  }

  vector<Transform> transforms;
  for (int i = 0; i < sphere_positions.size(); i++)
  {
    transforms.push_back(Transform(sphere_positions[i], quat(1,0,0,0),vec3(sphere_radiuses[i])));
  }

  Transform plane_transform(vec3(0,0,0),quat(1,0,0,0),vec3(200));

  float x = 0.0f;
  int width, height;
  float aspect;
  mat4 projection;

  glEnable(GL_DEPTH_TEST);
  while (!glfwWindowShouldClose(gfx.window()))
  {
    glfwGetFramebufferSize(gfx.window(), &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    aspect = (float)width/(float)height;
    projection = infinitePerspective(45.0f, aspect, 0.1f);

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

    pass->use();
    pass->updateMat4("view", camera.getView());
    pass->updateMat4("projection", projection);

    sphere_positions[0] = vec3(0,34+10*sin(x),0);
    transforms[0].set_translation(sphere_positions[0]);

    for (int j = 0; j < levels; j++)
    {
      GLfloat sx = (GLfloat)sin(x*2+(M_PI*0.25f*j));
      for (int i = 0; i < segments; i++)
      {
        sphere_positions[i+1+segments*j] = vec3(
            (50.0+20*sx) * cos(i * rad_per_lng + x*(0.1*j)),
            6.0+(12.0*j),
            (50.0+20*sx) * sin(i * rad_per_lng + x*(0.1*j))
          );
        transforms[i+1+segments*j].set_translation(sphere_positions[i+1+segments*j]);
      }
    }

    pass->updateFloatArray("radiuses", sphere_radiuses.data(), sphere_radiuses.size());
    pass->updateVec3Array("positions", sphere_positions.data(), sphere_positions.size());

    for (int i = 0; i < sphere_positions.size(); i++)
    {
      pass->updateMat4("world", transforms[i].world());
      pass->updateVec3("color", colors[i]);
      sph.draw();
    }

    pass->updateMat4("world", plane_transform.world());
    pass->updateVec3("color", vec3(1,1,1));
    pln.draw();

    glfwSwapBuffers(gfx.window());
    glfwPollEvents();
  }

  sph.destroy();
  pln.destroy();
  pass->destroy();
  delete pass;
  gfx.cleanup();

  delete [] sh_logs;
  return 0;
}
