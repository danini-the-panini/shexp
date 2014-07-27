#ifndef GFX_INCLUDE_H
#define GFX_INCLUDE_H

#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

using glm::cross;
using glm::dot;
using glm::lookAt;
using glm::mat4;
using glm::normalize;
using glm::ortho;
using glm::infinitePerspective;
using glm::rotate;
using glm::scale;
using glm::translate;
using glm::value_ptr;
using glm::vec3;
using glm::vec4;

using namespace std;

#endif
