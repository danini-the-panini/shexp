#ifndef GFX_INCLUDE_H
#define GFX_INCLUDE_H

#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

using glm::column;
using glm::cross;
using glm::detail::tmat4x4;
using glm::detail::tquat;
using glm::detail::tvec3;
using glm::detail::tvec4;
using glm::dot;
using glm::highp;
using glm::infinitePerspective;
using glm::lookAt;
using glm::mat4;
using glm::mat4_cast;
using glm::normalize;
using glm::ortho;
using glm::precision;
using glm::quat;
using glm::rotate;
using glm::scale;
using glm::translate;
using glm::value_ptr;
using glm::vec3;
using glm::vec4;

using namespace std;

#endif
