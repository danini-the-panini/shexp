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

int main(int argc, char** argv)
{
  int n = N_BANDS;
  ifstream in(argc > 1 ? argv[0] : "sh_lut.txt");
  if (!in)
  {
    cout << "Error loading file" << endl;
    return 1;
  }
  int lut_size;
  in >> lut_size;
  cout << lut_size << endl;
  float* sh_logs = new float[lut_size*n*n];

  double *light_coeff = new double[N_BANDS*N_BANDS];
  const int SQRT_N_SAMPLES = 1000;
  const int N_SAMPLES = SQRT_N_SAMPLES*SQRT_N_SAMPLES;
  SHSample* samples = new SHSample[N_SAMPLES];
  for (int i = 0; i < N_SAMPLES; i++)
  {
    samples[i].coeff = new double[N_COEFFS];
  }
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);

  SH_project_polar_function([&](double theta, double phi) {
      return 120000.0 * (1.0 + 2.0*sin(theta))/3.0;
    }, samples, N_SAMPLES, N_BANDS, light_coeff);

  const int CUBE_MAP_SIZE = 128;
  const int N_CUBE_MAPS = (N_BANDS*N_BANDS)/3+(N_BANDS%3?1:0);

  CubeMap h_maps[N_CUBE_MAPS];
  float h_data[N_CUBE_MAPS][6][CUBE_MAP_SIZE*CUBE_MAP_SIZE*3];

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
        double theta = acos(d[k].z);
        double phi = atan2(d[k].y,d[k].x);

        for(int l=0; l<N_BANDS; ++l) {
          for(int m=-l; m<=l; ++m) {
            int index = l*(l+1)+m;
            h_data[index/3][k][(i*CUBE_MAP_SIZE*3+j*3)+index%3] = (float)SH(l,m,theta,phi);
          }
        }
      }
    }
  }

  for (int i = 0; i < N_CUBE_MAPS; i++)
  {
    h_maps[i].build();
    h_maps[i].load_cube((const float **)(h_data[i]), CUBE_MAP_SIZE, CUBE_MAP_SIZE);
  }

  double tmp;
  for (int i = 0; i < lut_size*n*n; i++)
  {
    in >> tmp;
    cout << tmp << " ";
    sh_logs[i] = static_cast<float>(tmp);
  }
  cout << endl;

  for (int i = 0; i < lut_size; i++)
    sh_logs[i] = (float)sh_logs[i];

  GFXBoilerplate gfx;
  gfx.init();

  glEnable(GL_DEPTH_TEST);

  glfwSetCursorPosCallback(gfx.window(), mouse_callback);

  Shader* pass = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("sh_frag.glsl")
    ->build();

  char str[11];
  for (int i = 0; i < N_CUBE_MAPS; i++)
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
