#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  glm::uint32_t entityID;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDesc;
  }

  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDesc{};
    attributeDesc[0].binding = 0;
    attributeDesc[0].location = 0;
    attributeDesc[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
    attributeDesc[0].offset = offsetof(Vertex, pos);

    attributeDesc[1].binding = 0;
    attributeDesc[1].location = 1;
    attributeDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[1].offset = offsetof(Vertex, color);

    attributeDesc[2].binding = 0;
    attributeDesc[2].location = 2;
    attributeDesc[2].format = VK_FORMAT_R32_UINT;
    attributeDesc[2].offset = offsetof(Vertex, entityID);

    return attributeDesc;
  }
};

#endif
