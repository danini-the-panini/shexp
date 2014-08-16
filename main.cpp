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
  ////////////// LOAD SH LUT //////////////
  cout << "Loading SH LUT ... " << endl;

  int n = N_BANDS;
  ifstream in(argc > 1 ? argv[0] : "sh_lut.txt");
  if (!in)
  {
    cout << "Error loading file " << (argc > 1 ? argv[0] : "sh_lut.txt") << endl;
    return 1;
  }
  int lut_size;
  in >> lut_size;
  float* sh_logs = new float[lut_size*n*n];

  double tmp;
  for (int i = 0; i < lut_size*n*n; i++)
  {
    in >> tmp;
    sh_logs[i] = static_cast<float>(tmp);
  }

  for (int i = 0; i < lut_size; i++)
    sh_logs[i] = (float)sh_logs[i];

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

  const int CUBE_MAP_SIZE = 32;

  cout << "   - h_maps" << endl;
  CubeMap h_maps[N_COEFFS];
  cout << "   - h_data" << endl;
  float h_data[N_COEFFS][6][CUBE_MAP_SIZE*CUBE_MAP_SIZE];
  cout << " done." << endl;

  cout << " * Generating data ... \033[s";
  for (int i = 0; i < CUBE_MAP_SIZE; i++)
  {
    for (int j = 0; j < CUBE_MAP_SIZE; j++)
    {
      float u = ((float)j/(float)CUBE_MAP_SIZE)*2.0f-1.0f;
      float v = ((float)i/(float)CUBE_MAP_SIZE)*2.0f-1.0f;

      vec3 d[6] = {
        vec3( 1, -v, -u),
        vec3( u,  1,  v),
        vec3( u, -v,  1),
        vec3(-1, -v,  u),
        vec3( u, -1, -v),
        vec3(-u, -v, -1)
      };

      for (int k = 0; k < 6; k++)
      {
        cout << "\t( " << i << ", " << j << ", " << k << " )";

        double n_theta = acos(d[k].z);
        double n_phi = atan2(d[k].y,d[k].x);

        //SH_project_polar_function([&](double theta, double phi) {
            //return 120000.0 * (1.0 + 2.0*sin(theta))/3.0;
          //}, samples, N_SAMPLES, N_BANDS, light_coeff);

        //SH_project_polar_function([&](double s_theta, double s_phi) {
            //return dot_polar(s_theta, s_phi, n_theta, n_phi);
          //}, samples, N_SAMPLES, N_BANDS, h_coeff);

        //SH_product(light_coeff, h_coeff, l_coeff);

        for(int l=0; l<N_BANDS; ++l) {
          for(int m=-l; m<=l; ++m) {
            int index = l*(l+1)+m;
            h_data[index][k][i*CUBE_MAP_SIZE+j] = (float)i/(float)CUBE_MAP_SIZE;//(float)SH(l,m,n_theta,n_phi);
          }
        }

        //for(int index=0; index < N_COEFFS; ++index) {
          //h_data[index][k][i*CUBE_MAP_SIZE+j] = l_coeff[index];
        //}

        cout << "\033[u\033[K";
      }
    }
  }
  cout << "done." << endl;

  delete [] light_coeff;
  delete [] h_coeff;
  delete [] l_coeff;

  cout << " * Loading maps into textures ... ";
  for (int i = 0; i < N_COEFFS; i++)
  {
    h_maps[i].build();
    h_maps[i].load_cube((const float **)(h_data[i]), CUBE_MAP_SIZE, CUBE_MAP_SIZE,
        GL_R32F, GL_RED, GL_FLOAT);
  }
  cout << "done." << endl;

  cout << "done." << endl;

  ///////////////// DO THE OPENGL THING ////////////////
  GFXBoilerplate gfx;
  gfx.init();

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

  char str[11];
  for (int i = 0; i < N_COEFFS; i++)
  {
    sprintf(str,"h_maps[%d]", i);
    pass->updateInt(str, i);
    h_maps[i].use(GL_TEXTURE0+i);
  }

  Sphere sph;
  sph.build();

  Plane pln;
  pln.build();

  pass->use();
  pass->updateMat4("projection",
      infinitePerspective(45.0f, 640.0f/480.0f, 0.1f));
  pass->updateFloatArray("sh_lut", sh_logs, lut_size*n*n);

  vector<vec3> sphere_positions;
  vector<GLfloat> sphere_radiuses;
  vector<vec3> colors;

  sphere_positions.push_back(vec3(0,20,0));
  sphere_radiuses.push_back(20.0f);
  colors.push_back(vec3(0,1,1));

  const GLuint segments = 16;
  const GLfloat rad_per_lng = (2.f*(GLfloat)M_PI) / (GLfloat)segments;
  for (int i = 0; i < segments; i++)
  {
    sphere_positions.push_back(vec3(
          50.0 * cos(i * rad_per_lng),
          6.0,
          50.0 * sin(i * rad_per_lng)
          ));
    sphere_radiuses.push_back(6.0f);
    colors.push_back(vec3(1,0,1));
  }

  vector<Transform> transforms;
  for (int i = 0; i < sphere_positions.size(); i++)
  {
    transforms.push_back(Transform(sphere_positions[i], quat(1,0,0,0),vec3(sphere_radiuses[i])));
  }

  Transform plane_transform(vec3(0,0,0),quat(1,0,0,0),vec3(200));

  float x = 0.0f;

  while (!glfwWindowShouldClose(gfx.window()))
  {
    x += 0.01f;

    handleInput(gfx.window());
    pass->updateMat4("view", camera.getView());

    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sphere_positions[0] = vec3(0,30+10*sin(x),0);
    transforms[0].set_translation(sphere_positions[0]);

    GLfloat sx = (GLfloat)sin(x*2);
    for (int i = 0; i < segments; i++)
    {
      sphere_positions[i+1] = vec3(
          (50.0+20*sx) * cos(i * rad_per_lng),
          6.0,
          (50.0+20*sx) * sin(i * rad_per_lng)
        );
      transforms[i+1].set_translation(sphere_positions[i+1]);
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
