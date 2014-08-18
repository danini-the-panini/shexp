#include <cstdlib>
#include <iostream>
#include <fstream>

#include "gfx_boilerplate.h"
#include "sh_functions.h"
#include "shader.h"
#include "sphere.h"
#include "plane.h"
#include "camera.h"
#include "transform.h"
#include "texture2d.h"
#include "cube_map.h"
#include "green.h"

using namespace std;

typedef OrthoRotMatCamera<float, highp> ORMCamF;
ORMCamF camera(ORMCamF::vec3_type(0,30,-50));

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

template < typename T >
T dot_polar(T theta1, T phi1, T theta2, T phi2)
{
  return sin(phi1) * sin(phi2) * cos(theta1-theta2) + cos(phi1) * cos(phi2);
}

int main(int argc, char** argv)
{
  GFXBoilerplate gfx;
  gfx.init();

  ////////////// LOAD SH LUT //////////////
  cout << "Loading SH LUT ... ";

  Texture2d sh_lut;

  ifstream in(argc > 1 ? argv[0] : "sh_lut.txt");
  if (!in)
  {
    cout << "Error loading file " << (argc > 1 ? argv[0] : "sh_lut.txt") << endl;
    return 1;
  }
  int lut_size;
  in >> lut_size;
  float* sh_logs = new float[lut_size*N_COEFFS];

  double tmp;
  for (int i = 0; i < lut_size*N_COEFFS; i++)
  {
    in >> tmp;
    sh_logs[i] = static_cast<float>(tmp);
  }

  for (int i = 0; i < lut_size; i++)
    sh_logs[i] = (float)sh_logs[i];

  glActiveTexture(GL_TEXTURE0);
  sh_lut.build();
  sh_lut.load_tex(sh_logs, N_COEFFS, lut_size, GL_R32F, GL_RED, GL_FLOAT);

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
  cout << " * Generating SH samples ... ";
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);
  cout << "done." << endl;

  cout << " * Allocating ... " << endl;
  cout << "   - light_coeff" << endl;
  double *light_coeff = new double[N_COEFFS];
  cout << "   - h_coeff" << endl;
  double *h_coeff = new double[N_COEFFS];
  cout << "   - l_coeff" << endl;
  double *l_coeff = new double[N_COEFFS];

  SH_project_polar_function([&](double theta, double phi) {
      return 1;//(1.0 + 2.0*sin(theta))/3.0;
    }, samples, N_SAMPLES, N_BANDS, light_coeff);

  const int CUBE_MAP_SIZE = 8;

  cout << "   - h_maps" << endl;
  CubeMap h_maps[N_COEFFS];
  cout << "   - h_data" << endl;
  float **h_data[N_COEFFS];
  for (int i = 0; i < N_COEFFS; i++)
  {
    h_data[i] = new float*[6];
    for (int j = 0; j < 6; j++)
    {
      h_data[i][j] = new float[CUBE_MAP_SIZE*CUBE_MAP_SIZE];
    }
  }
  cout << " done." << endl;

  cout << " * Generating data ... \033[s";
  float GU = CUBE_MAP_SIZE*0.5f;
  for (int i = 0; i < CUBE_MAP_SIZE; i++)
  {
    for (int j = 0; j < CUBE_MAP_SIZE; j++)
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
        cout << "( " << i << ", " << j << ", " << k << " )";

        double x = d[k].x, y = d[k].y, z = d[k].z;

        double n_theta = (M_PI*0.5) - atan2(y, sqrt(x*x+z*z));
        double n_phi = atan2(z, x);

        //SH_project_polar_function([&](double s_theta, double s_phi) {
            //return max(dot_polar(s_theta, s_phi, n_theta, n_phi), 0.0);
          //}, samples, N_SAMPLES, N_BANDS, h_coeff);

        //for (int h = 0; h < N_COEFFS; h++)
        //{
          //h_coeff[h] /= M_PI;
        //}

        //SH_product(light_coeff, h_coeff, l_coeff);

        //for(int index=0; index < N_COEFFS; ++index) {
          //h_data[index][k][i*CUBE_MAP_SIZE+j] = l_coeff[index];
        //}

        for(int l=0; l<N_BANDS; ++l) {
          for(int m=-l; m<=l; ++m) {
            int index = l*(l+1)+m;
            h_data[index][k][i*CUBE_MAP_SIZE+j] = (float)SH(l,m,n_theta,n_phi);
          }
        }

        cout << "\033[u\033[K";
      }
    }
  }
  cout << "done." << endl;

  cout << " * Loading maps into textures ... ";
  for (int i = 0; i < N_COEFFS; i++)
  {
    glActiveTexture(GL_TEXTURE1+i);
    h_maps[i].build();
    h_maps[i].load_cube(h_data[i], CUBE_MAP_SIZE, CUBE_MAP_SIZE,
        GL_R32F, GL_RED, GL_FLOAT);
  }
  cout << "done." << endl;

  delete [] light_coeff;
  delete [] h_coeff;
  delete [] l_coeff;
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

  glEnable(GL_DEPTH_TEST);

  glfwSetCursorPosCallback(gfx.window(), mouse_callback);

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

  pass->use();
  int indices[N_COEFFS];
  for (int i = 0; i < N_COEFFS; i++)
  {
    indices[i] = 1+i;
  }
  pass->updateInt("sh_lut", 0);
  pass->updateInts("h_maps", indices, N_COEFFS);

  vector<vec3> sphere_positions;
  vector<GLfloat> sphere_radiuses;
  vector<vec3> colors;

  sphere_positions.push_back(vec3(0,20,0));
  sphere_radiuses.push_back(20.0f);
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

  while (!glfwWindowShouldClose(gfx.window()))
  {
    x += 0.01f;

    handleInput(gfx.window());
    pass->updateMat4("view", camera.getView());

    glfwGetFramebufferSize(gfx.window(), &width, &height);
    pass->updateMat4("projection",
        infinitePerspective(45.0f, (float)width/(float)height, 0.1f));
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sphere_positions[0] = vec3(0,30+10*sin(x),0);
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
