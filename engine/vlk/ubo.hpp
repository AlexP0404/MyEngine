#ifndef UBO_HPP
#define UBO_HPP

/* #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES */
#include <glm/glm.hpp>

struct UBO {
  glm::vec3 worldPos[400];
  glm::mat4 viewMat;
};

#endif // !UBO_HPP
