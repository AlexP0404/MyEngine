#ifndef CIRCLE_VERTEX_HPP
#define CIRCLE_VERTEX_HPP

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct CircleVertex {
  glm::vec2 worldPos;
  glm::vec2 localPos;
  glm::vec3 color;
  float radius;

  glm::uint32_t entityID;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(CircleVertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDesc;
  }

  static std::array<VkVertexInputAttributeDescription, 5>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 5> attributeDesc{};
    attributeDesc[0].binding = 0;
    attributeDesc[0].location = 0;
    attributeDesc[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
    attributeDesc[0].offset = offsetof(CircleVertex, worldPos);

    attributeDesc[1].binding = 0;
    attributeDesc[1].location = 1;
    attributeDesc[1].format = VK_FORMAT_R32G32_SFLOAT; // vec2
    attributeDesc[1].offset = offsetof(CircleVertex, localPos);

    attributeDesc[2].binding = 0;
    attributeDesc[2].location = 2;
    attributeDesc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[2].offset = offsetof(CircleVertex, color);

    attributeDesc[3].binding = 0;
    attributeDesc[3].location = 3;
    attributeDesc[3].format = VK_FORMAT_R32_SFLOAT;
    attributeDesc[3].offset = offsetof(CircleVertex, radius);

    attributeDesc[4].binding = 0;
    attributeDesc[4].location = 4;
    attributeDesc[4].format = VK_FORMAT_R32_UINT;
    attributeDesc[4].offset = offsetof(CircleVertex, entityID);

    return attributeDesc;
  }
};

#endif
