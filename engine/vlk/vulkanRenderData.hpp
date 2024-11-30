#ifndef VULKAN_RENDER_DATA_HPP
#define VULKAN_RENDER_DATA_HPP

#include "circleVertex.hpp"
#include "ubo.hpp"
#include "vertex.hpp"
#include "vulkanInit.hpp"

#include <cstdint>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanRenderData {
public:
  explicit VulkanRenderData();
  ~VulkanRenderData();
  void initRenderData(std::shared_ptr<VulkanInit> pInit);
  void prepareFrame(); // used for IMGUI preparing
  void drawFrame(bool pFrameBufferResized);
  void devWaitIdle();
  // these are used by the renderer class

  void updateEntityPos(uint32_t pEntityID, glm::vec2 pCurrentPos);
  void initNewEntity(bool pIsCircle = false);
  void drawIndexed(uint32_t pNumQuadsToDraw = 0, uint32_t pNumCircsToDraw = 0);

  // these should be changed to builder functions that take in a location,
  // and size/radius and maybe ID
  // void insertQuad(const std::array<Vertex, 4> &pVertices,
  //                 const uint32_t pQuadNum);
  // void insertCircle(const std::array<CircleVertex, 4> &pVertices,
  //                   const uint32_t pCircleNum);

  void clearVertices();

  const int MAX_FRAMES_IN_FLIGHT = 2;
  const int MAX_QUAD_COUNT = 400;
  const int MAX_VERTEX_COUNT = MAX_QUAD_COUNT * 4;
  const int MAX_INDEX_COUNT = MAX_QUAD_COUNT * 6;
  std::vector<Vertex> mQuadVertices;
  std::vector<CircleVertex> mCircleVertices;

private:
  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;

  std::vector<VkImage> mSwapChainImages;
  std::vector<VkImageView> mSwapChainImageViews;
  std::vector<VkFramebuffer> mFramebuffers;

  VkRenderPass mRenderPassInit;
  VkRenderPass mRenderPassFinal;

  VkDescriptorSetLayout mDescSetLayout;
  VkDescriptorPool mDescPool;
  std::vector<VkDescriptorSet> mDescSets;

  VkPipelineLayout mQuadPipelineLayout;
  VkPipeline mQuadGraphicsPipeline;
  VkPipelineLayout mCirclePipelineLayout;
  VkPipeline mCirclePipeline;

  VkCommandPool mCommandPool;
  std::vector<VkCommandBuffer> mCommandBuffers;

  VkBuffer mQuadVertexBuffer;
  VkDeviceMemory mQuadVertexBufferMem;
  VkBuffer mQuadIndexBuffer;
  VkDeviceMemory mQuadIndexBuferMem;
  std::vector<uint16_t> mQuadIndices;
  VkBuffer mCircleIndexBuffer;
  VkDeviceMemory mCircleIndexBuferMem;
  std::vector<uint16_t> mCircleIndices;

  VkBuffer mCircVertexBuffer;
  VkDeviceMemory mCircVertexBufferMem;

  std::vector<VkBuffer> mUniformBuffers;
  std::vector<VkDeviceMemory> mUniformBuffersMemory;
  std::vector<void *> mUniformBuffersMapped;
  UBO mUbo;

  std::vector<VkSemaphore> mImgAvailSemaphores;
  std::vector<VkSemaphore> mRndrFinSemaphores;
  std::vector<VkFence> mInFlightFences;

  // TODO MAKE AN IMGUI DRAWING FUNCTION SIMILAR TO recordXCommandBuffer(...)
  // FUNCTIONS THAT ARE ALREADY THERE

private:
  void initQueues();
  void getSwapChainImages();
  void createImageViews();
  void recreateSwapchain();
  void createRenderPasses();
  void createDescriptorSetLayout();
  void createGraphicsPipeline(
      std::string_view pVertName, std::string_view pFragName,
      VkVertexInputBindingDescription pBindingDesc, size_t pAttrDescCount,
      VkVertexInputAttributeDescription *ppAttrDesc,
      uint32_t pDescSetLayoutCount, VkDescriptorSetLayout *ppDescSetLayout,
      VkPipelineLayout &pPipelineLayout, VkPipeline &pPipeline);
  void createFramebuffers();
  void createCommandPool();
  void createBuffer(VkDeviceSize pSize, VkBufferUsageFlags pUsage,
                    VkMemoryPropertyFlags pProperties, VkBuffer &pBuffer,
                    VkDeviceMemory &pBufferMem);
  void copyBuffer(VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize pSize);
  void createVertexBuffers();
  template <typename t>
  void createVertexBuffer(std::vector<t> pVertices, VkBuffer &pBuffer,
                          VkDeviceMemory &pBufferMem);
  void createIndexBuffers();
  void createIndexBuffer(std::vector<uint16_t> pIndices, VkBuffer &pBuffer,
                         VkDeviceMemory &pBufferMem);
  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();
  void updateUniformBuffer(uint32_t pCurrentImage);
  uint32_t findMemoryType(uint32_t pTypeFilter,
                          VkMemoryPropertyFlags pProperties);
  void createCommandBuffers();
  void createCommandBuffers(size_t pBeginIndex, size_t pEndIndex);
  void recordQuadCommandBuffer(VkCommandBuffer pCommandBuffer,
                               uint32_t pImageIndex);
  void recordCircCommandBuffer(VkCommandBuffer pCommandBuffer,
                               uint32_t pImageIndex);
  void recordImGuiCommandBuffer(VkCommandBuffer pCommandBuffer,
                                uint32_t pImageIndex);
  void createSyncObjects();
  void initImGUI();

  static std::vector<char> readShader(std::string_view pFilename);
  VkShaderModule createShaderModule(const std::vector<char> &pCode);

private:
  std::shared_ptr<VulkanInit> mInit;
  uint32_t mCurrentFrame;
  uint32_t mNumQuadIndicesToDraw;
  uint32_t mNumCirclesIndicesToDraw;
};

#endif
