#include "vulkanRenderData.hpp"

#include "circleVertex.hpp"
#include "utils.hpp"
#include "vertex.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <array>
#include <cassert>
#include <fstream>
#include <stdexcept>

static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

VulkanRenderData::VulkanRenderData() {
  mCurrentFrame = 0;
  mNumQuadIndicesToDraw = 0;
  mNumCirclesIndicesToDraw = 0;
}

VulkanRenderData::~VulkanRenderData() {
  assert(mInit);
  //
  //
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(mInit->mLogicalDevice, mUniformBuffers[i], nullptr);
    vkFreeMemory(mInit->mLogicalDevice, mUniformBuffersMemory[i], nullptr);
  }
  vkDestroyDescriptorPool(mInit->mLogicalDevice, mDescPool, nullptr);
  vkDestroyDescriptorSetLayout(mInit->mLogicalDevice, mDescSetLayout, nullptr);

  // vkDestroyBuffer(mInit->mLogicalDevice, mCircleIndexBuffer, nullptr);
  // vkFreeMemory(mInit->mLogicalDevice, mCircleIndexBuferMem, nullptr);
  vkDestroyBuffer(mInit->mLogicalDevice, mCircVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mCircVertexBufferMem, nullptr);

  vkDestroyBuffer(mInit->mLogicalDevice, mCubeIndexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mCubeIndexBuferMem, nullptr);
  vkDestroyBuffer(mInit->mLogicalDevice, mCubeVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mCubeVertexBufferMem, nullptr);

  vkDestroyBuffer(mInit->mLogicalDevice, mQuadIndexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mQuadIndexBuferMem, nullptr);
  vkDestroyBuffer(mInit->mLogicalDevice, mQuadVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mQuadVertexBufferMem, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(mInit->mLogicalDevice, mImgAvailSemaphores[i], nullptr);
    vkDestroySemaphore(mInit->mLogicalDevice, mRndrFinSemaphores[i], nullptr);
    vkDestroyFence(mInit->mLogicalDevice, mInFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(mInit->mLogicalDevice, mCommandPool, nullptr);

  for (auto framebuffer : mFramebuffers) {
    vkDestroyFramebuffer(mInit->mLogicalDevice, framebuffer, nullptr);
  }
  vkDestroyPipeline(mInit->mLogicalDevice, mCirclePipeline, nullptr);
  vkDestroyPipelineLayout(mInit->mLogicalDevice, mCirclePipelineLayout,
                          nullptr);
  vkDestroyPipeline(mInit->mLogicalDevice, mQuadGraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(mInit->mLogicalDevice, mQuadPipelineLayout, nullptr);
  vkDestroyRenderPass(mInit->mLogicalDevice, mRenderPassInit, nullptr);
  vkDestroyRenderPass(mInit->mLogicalDevice, mRenderPassFinal, nullptr);
  for (auto imageView : mSwapChainImageViews)
    vkDestroyImageView(mInit->mLogicalDevice, imageView, nullptr);
}

void VulkanRenderData::initRenderData(std::shared_ptr<VulkanInit> pInit) {
  mInit = pInit;
  initQueues();
  getSwapChainImages();
  createImageViews();
  createRenderPasses();
  createDescriptorSetLayout();
  createGraphicsPipeline("shaders/quadVert.spv", "shaders/quadFrag.spv",
                         Vertex::getBindingDescription(),
                         Vertex::getAttributeDescriptions().size(),
                         Vertex::getAttributeDescriptions().data(), 1,
                         &mDescSetLayout, mQuadPipelineLayout,
                         mQuadGraphicsPipeline);
  createGraphicsPipeline("shaders/circleVert.spv", "shaders/circleFrag.spv",
                         CircleVertex::getBindingDescription(),
                         CircleVertex::getAttributeDescriptions().size(),
                         CircleVertex::getAttributeDescriptions().data(), 1,
                         &mDescSetLayout, mCirclePipelineLayout,
                         mCirclePipeline);
  createFramebuffers();
  createCommandPool();
  createVertexBuffers();
  createIndexBuffers();
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
  createCommandBuffers();
  createSyncObjects();
  initImGUI();
}

void VulkanRenderData::devWaitIdle() {
  vkDeviceWaitIdle(mInit->mLogicalDevice);
}

void VulkanRenderData::initQueues() {
  assert(mInit); // make sure vulkan init has completed and been set
  vkGetDeviceQueue(mInit->mLogicalDevice,
                   mInit->mQFIndices.graphicsFamily.value(), 0,
                   &mGraphicsQueue);
  vkGetDeviceQueue(mInit->mLogicalDevice,
                   mInit->mQFIndices.graphicsFamily.value(), 0, &mPresentQueue);
}

void VulkanRenderData::getSwapChainImages() {
  assert(mInit);
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(mInit->mLogicalDevice, mInit->mSwapChain, &imageCount,
                          nullptr);
  mSwapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mInit->mLogicalDevice, mInit->mSwapChain, &imageCount,
                          mSwapChainImages.data());
}

void VulkanRenderData::createImageViews() {
  assert(mInit);
  mSwapChainImageViews.resize(mSwapChainImages.size());

  for (size_t i = 0; i < mSwapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    Utils::zeroInitializeStruct(createInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = mSwapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = mInit->mSwapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(mInit->mLogicalDevice, &createInfo, nullptr,
                          &mSwapChainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image views!");
    }
  }
}

void VulkanRenderData::createRenderPasses() {
  VkAttachmentDescription colorAttachmentInit{};
  Utils::zeroInitializeStruct(colorAttachmentInit);
  colorAttachmentInit.format = mInit->mSwapChainImageFormat;
  colorAttachmentInit.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentInit.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentInit.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentInit.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachmentInit.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachmentInit.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentInit.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  Utils::zeroInitializeStruct(colorAttachmentRef);
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  Utils::zeroInitializeStruct(subpass);
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  // THIS IS (LAYOUT = 0) FROM THE FRAGMENT SHADER CODE
  // change below to read in more data from fragment shader
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  Utils::zeroInitializeStruct(renderPassInfo);
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachmentInit;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (vkCreateRenderPass(mInit->mLogicalDevice, &renderPassInfo, nullptr,
                         &mRenderPassInit) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass!");
  }

  VkAttachmentDescription colorAttachmentFinal{};
  colorAttachmentFinal = colorAttachmentInit;
  colorAttachmentFinal.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  colorAttachmentFinal.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkRenderPassCreateInfo renderPassInfo2{};
  renderPassInfo2 = renderPassInfo;
  renderPassInfo2.pAttachments = &colorAttachmentFinal;

  if (vkCreateRenderPass(mInit->mLogicalDevice, &renderPassInfo2, nullptr,
                         &mRenderPassFinal) != VK_SUCCESS)
    throw std::runtime_error("Failed to create second render pass!");
}

void VulkanRenderData::createDescriptorSetLayout() {
  assert(mInit);
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  Utils::zeroInitializeStruct(uboLayoutBinding);
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  Utils::zeroInitializeStruct(layoutInfo);
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  VK_CHECK(vkCreateDescriptorSetLayout(mInit->mLogicalDevice, &layoutInfo,
                                       nullptr, &mDescSetLayout));
}

void VulkanRenderData::createGraphicsPipeline(
    std::string_view pVertName, std::string_view pFragName,
    VkVertexInputBindingDescription pBindingDesc, size_t pAttrDescCount,
    VkVertexInputAttributeDescription *ppAttrDesc, uint32_t pDescSetLayoutCount,
    VkDescriptorSetLayout *ppDescSetLayout, VkPipelineLayout &pPipelineLayout,
    VkPipeline &pPipeline) {
  assert(mInit);
  auto vertShaderCode = readShader(pVertName);
  auto fragShaderCode = readShader(pFragName);

  VkShaderModule vShaderModule = createShaderModule(vertShaderCode);
  VkShaderModule fShaderModule = createShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  Utils::zeroInitializeStruct(vertShaderStageInfo);
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  // sending shader to vertex stage of pipeline
  vertShaderStageInfo.module = vShaderModule; // set module
  vertShaderStageInfo.pName = "main";         // tell what function to read 1st

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  Utils::zeroInitializeStruct(fragShaderStageInfo);
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  Utils::zeroInitializeStruct(vertexInputInfo);

  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &pBindingDesc;
  vertexInputInfo.vertexAttributeDescriptionCount = pAttrDescCount;
  vertexInputInfo.pVertexAttributeDescriptions = ppAttrDesc;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  Utils::zeroInitializeStruct(inputAssembly);
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  Utils::zeroInitializeStruct(viewport);
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)mInit->mSwapChainExtent.width;
  viewport.height = (float)mInit->mSwapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  Utils::zeroInitializeStruct(scissor);
  scissor.offset = {0, 0};
  scissor.extent = mInit->mSwapChainExtent;

  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  Utils::zeroInitializeStruct(dynamicState);
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineViewportStateCreateInfo viewportState{};
  Utils::zeroInitializeStruct(viewportState);
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  Utils::zeroInitializeStruct(rasterizer);
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  Utils::zeroInitializeStruct(multisampling);
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  Utils::zeroInitializeStruct(colorBlendAttachment);
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  Utils::zeroInitializeStruct(colorBlending);
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  Utils::zeroInitializeStruct(pipelineLayoutInfo);
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = pDescSetLayoutCount;
  pipelineLayoutInfo.pSetLayouts = ppDescSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(mInit->mLogicalDevice, &pipelineLayoutInfo,
                             nullptr, &pPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  Utils::zeroInitializeStruct(pipelineInfo);
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pPipelineLayout;
  pipelineInfo.renderPass = mRenderPassInit;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(mInit->mLogicalDevice, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &pPipeline) != VK_SUCCESS)
    throw std::runtime_error("Failed to create graphics pipeline!");

  vkDestroyShaderModule(mInit->mLogicalDevice, vShaderModule, nullptr);
  vkDestroyShaderModule(mInit->mLogicalDevice, fShaderModule, nullptr);
}

std::vector<char> VulkanRenderData::readShader(std::string_view pFilename) {
  std::ifstream file(pFilename.data(), std::ios::ate | std::ios::binary);
  // open back of file and read in binary format

  if (!file.is_open()) { // has to be valid file
    throw std::runtime_error("Failed to open file! Given: " +
                             std::string(pFilename));
  }
  // end position gives file size in bytes
  size_t fileSize = (size_t)file.tellg();
  // allocate buffer of that size
  std::vector<char> buffer(fileSize);
  // go back to beginning
  file.seekg(0);
  // read in bytes to buffer
  file.read(buffer.data(), fileSize);
  // close file
  file.close();

  return buffer;
}

VkShaderModule
VulkanRenderData::createShaderModule(const std::vector<char> &pCode) {
  assert(mInit);
  VkShaderModuleCreateInfo createInfo{};
  Utils::zeroInitializeStruct(createInfo);
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = pCode.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(pCode.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(mInit->mLogicalDevice, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module!");
  }
  return shaderModule;
}

void VulkanRenderData::createFramebuffers() {
  assert(mInit);
  mFramebuffers.resize(mSwapChainImageViews.size());
  for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
    VkImageView attachments[] = {mSwapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    Utils::zeroInitializeStruct(framebufferInfo);
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = mRenderPassInit;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = mInit->mSwapChainExtent.width;
    framebufferInfo.height = mInit->mSwapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(mInit->mLogicalDevice, &framebufferInfo, nullptr,
                            &mFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

void VulkanRenderData::createCommandPool() {
  assert(mInit);
  VkCommandPoolCreateInfo poolInfo{};
  Utils::zeroInitializeStruct(poolInfo);
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = mInit->mQFIndices.graphicsFamily.value();

  if (vkCreateCommandPool(mInit->mLogicalDevice, &poolInfo, nullptr,
                          &mCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool!");
  }
}

void VulkanRenderData::createBuffer(VkDeviceSize pSize,
                                    VkBufferUsageFlags pUsage,
                                    VkMemoryPropertyFlags pProperties,
                                    VkBuffer &pBuffer,
                                    VkDeviceMemory &pBufferMem) {
  assert(mInit);
  VkBufferCreateInfo bufferInfo{};
  Utils::zeroInitializeStruct(bufferInfo);
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = pSize;
  bufferInfo.usage = pUsage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(mInit->mLogicalDevice, &bufferInfo, nullptr, &pBuffer) !=
      VK_SUCCESS)
    throw std::runtime_error("Failed to create vertex buffer!");

  VkMemoryRequirements memRequ;
  vkGetBufferMemoryRequirements(mInit->mLogicalDevice, pBuffer, &memRequ);

  VkMemoryAllocateInfo allocInfo{};
  Utils::zeroInitializeStruct(allocInfo);
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequ.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequ.memoryTypeBits, pProperties);

  if (vkAllocateMemory(mInit->mLogicalDevice, &allocInfo, nullptr,
                       &pBufferMem) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate vertex buffer memory!");

  vkBindBufferMemory(mInit->mLogicalDevice, pBuffer, pBufferMem, 0);
}

void VulkanRenderData::copyBuffer(VkBuffer pSrcBuffer, VkBuffer pDstBuffer,
                                  VkDeviceSize pSize) {
  assert(mInit);
  VkCommandBufferAllocateInfo allocInfo{};
  Utils::zeroInitializeStruct(allocInfo);
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = mCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(mInit->mLogicalDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  Utils::zeroInitializeStruct(beginInfo);
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  Utils::zeroInitializeStruct(copyRegion);
  copyRegion.size = pSize;
  vkCmdCopyBuffer(commandBuffer, pSrcBuffer, pDstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  Utils::zeroInitializeStruct(submitInfo);
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(mGraphicsQueue);

  vkFreeCommandBuffers(mInit->mLogicalDevice, mCommandPool, 1, &commandBuffer);
}

void VulkanRenderData::createVertexBuffers() {
  mCubeVertices.resize(MAX_VERTEX_COUNT);
  mQuadVertices.resize(MAX_VERTEX_COUNT);
  mCircleVertices.resize(MAX_VERTEX_COUNT);
  createVertexBuffer(mCubeVertices, mCubeVertexBuffer, mCubeVertexBufferMem);
  createVertexBuffer(mQuadVertices, mQuadVertexBuffer, mQuadVertexBufferMem);
  createVertexBuffer(mCircleVertices, mCircVertexBuffer, mCircVertexBufferMem);
}

template <typename t>
void VulkanRenderData::createVertexBuffer(std::vector<t> pVertices,
                                          VkBuffer &pBuffer,
                                          VkDeviceMemory &pBufferMem) {
  VkDeviceSize bufferSize = sizeof(pVertices[0]) * pVertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBuffMem;

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBuffMem);

  void *data;
  vkMapMemory(mInit->mLogicalDevice, stagingBuffMem, 0, bufferSize, 0, &data);
  memcpy(data, pVertices.data(), (size_t)bufferSize);
  vkUnmapMemory(mInit->mLogicalDevice, stagingBuffMem);

  createBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pBuffer, pBufferMem);

  copyBuffer(stagingBuffer, pBuffer, bufferSize);

  vkDestroyBuffer(mInit->mLogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, stagingBuffMem, nullptr);
}

uint32_t VulkanRenderData::findMemoryType(uint32_t pTypeFilter,
                                          VkMemoryPropertyFlags pProperties) {
  assert(mInit);
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(mInit->mPhysicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (pTypeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
                                   pProperties) == pProperties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

void VulkanRenderData::createIndexBuffers() {
  // fill index array (may be wasteful idk)
  mQuadIndices.resize(MAX_INDEX_COUNT);
  mCubeIndices.resize(MAX_CUBE_IDX_COUNT);
  uint32_t offset = 0;
  for (size_t i = 0; i < MAX_INDEX_COUNT; i += 6) {
    mQuadIndices[i + 0] = 0 + offset;
    mQuadIndices[i + 1] = 1 + offset;
    mQuadIndices[i + 2] = 2 + offset;
    mQuadIndices[i + 3] = 2 + offset;
    mQuadIndices[i + 4] = 3 + offset;
    mQuadIndices[i + 5] = 0 + offset;

    offset += 4;
  }
  offset = 0;
  std::vector<uint16_t> singleCube = {// Top
                                      2, 6, 7, 2, 3, 7,

                                      // Bottom
                                      0, 4, 5, 0, 1, 5,

                                      // Left
                                      0, 2, 6, 0, 4, 6,

                                      // Right
                                      1, 3, 7, 1, 5, 7,

                                      // Front
                                      0, 2, 3, 0, 1, 3,

                                      // Back
                                      4, 6, 7, 4, 5, 7};
  for (int i = 0; i < MAX_CUBE_IDX_COUNT; i += 6 * 6) {
    for (auto &idx : singleCube) {
      idx += offset;
    }
    mCubeIndices.insert(mCubeIndices.end(), singleCube.begin(),
                        singleCube.end());
    offset += 8;
  }
  // mCircleIndices = mQuadIndices;

  createIndexBuffer(mQuadIndices, mQuadIndexBuffer, mQuadIndexBuferMem);
  createIndexBuffer(mCubeIndices, mCubeIndexBuffer, mCubeIndexBuferMem);
  // createIndexBuffer(mCircleIndices, mCircleIndexBuffer,
  // mCircleIndexBuferMem);
}

void VulkanRenderData::createIndexBuffer(std::vector<uint16_t> pIndices,
                                         VkBuffer &pBuffer,
                                         VkDeviceMemory &pBufferMem) {
  assert(mInit);
  VkDeviceSize bufferSize = sizeof(pIndices[0]) * pIndices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBuffMem;

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBuffMem);

  void *data;
  vkMapMemory(mInit->mLogicalDevice, stagingBuffMem, 0, bufferSize, 0, &data);
  memcpy(data, pIndices.data(), (size_t)bufferSize);
  vkUnmapMemory(mInit->mLogicalDevice, stagingBuffMem);

  createBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pBuffer, pBufferMem);

  copyBuffer(stagingBuffer, pBuffer, bufferSize);

  vkDestroyBuffer(mInit->mLogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, stagingBuffMem, nullptr);
}

void VulkanRenderData::createUniformBuffers() {
  assert(mInit);
  VkDeviceSize bufferSize = sizeof(UBO);

  mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  mUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  mUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 mUniformBuffers[i], mUniformBuffersMemory[i]);
    vkMapMemory(mInit->mLogicalDevice, mUniformBuffersMemory[i], 0, bufferSize,
                0, &mUniformBuffersMapped[i]);
  }
}

void VulkanRenderData::createDescriptorPool() {
  VkDescriptorPoolSize poolSize[] = {
      poolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      poolSize[0].descriptorCount = 1,
      // static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)};
      poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      poolSize[1].descriptorCount =
          static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)};
  VkDescriptorPoolCreateInfo poolInfo{};
  Utils::zeroInitializeStruct(poolInfo);
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.poolSizeCount =
      static_cast<uint32_t>(sizeof(poolSize) / sizeof(VkDescriptorPoolSize));
  poolInfo.pPoolSizes = poolSize;
  poolInfo.maxSets = 1000; // THIS MAY NOT BE CORRECT

  VK_CHECK(vkCreateDescriptorPool(mInit->mLogicalDevice, &poolInfo, nullptr,
                                  &mDescPool));
}

void VulkanRenderData::createDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             mDescSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  Utils::zeroInitializeStruct(allocInfo);
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = mDescPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  mDescSets.resize(MAX_FRAMES_IN_FLIGHT);
  VK_CHECK(vkAllocateDescriptorSets(mInit->mLogicalDevice, &allocInfo,
                                    mDescSets.data()));

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    Utils::zeroInitializeStruct(bufferInfo);
    bufferInfo.buffer = mUniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UBO);

    VkWriteDescriptorSet descWrite{};
    Utils::zeroInitializeStruct(descWrite);
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.dstSet = mDescSets[i];
    descWrite.dstBinding = 0;
    descWrite.dstArrayElement = 0;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descWrite.descriptorCount = 1;
    descWrite.pBufferInfo = &bufferInfo;
    descWrite.pImageInfo = nullptr;
    descWrite.pTexelBufferView = nullptr;
    vkUpdateDescriptorSets(mInit->mLogicalDevice, 1, &descWrite, 0, nullptr);
  }
}

void VulkanRenderData::createCommandBuffers(size_t pBeginIndex,
                                            size_t pEndIndex) {
  VkCommandBufferAllocateInfo allocInfo{};
  Utils::zeroInitializeStruct(allocInfo);
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = mCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = pEndIndex - pBeginIndex;

  if (vkAllocateCommandBuffers(mInit->mLogicalDevice, &allocInfo,
                               mCommandBuffers.data() + pBeginIndex) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create command buffer!");
  }
}

void VulkanRenderData::createCommandBuffers() {
  mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT * 3);
  VkCommandBufferAllocateInfo allocInfo{};
  Utils::zeroInitializeStruct(allocInfo);
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = mCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

  if (vkAllocateCommandBuffers(mInit->mLogicalDevice, &allocInfo,
                               mCommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command buffer!");
  }
}

void VulkanRenderData::recordQuadCommandBuffer(VkCommandBuffer pCommandBuffer,
                                               uint32_t pImageIndex) {
  assert(mInit);
  VkCommandBufferBeginInfo beginInfo{};
  Utils::zeroInitializeStruct(beginInfo);
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(pCommandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  Utils::zeroInitializeStruct(renderPassInfo);
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = mRenderPassInit;
  renderPassInfo.framebuffer = mFramebuffers[pImageIndex];
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = mInit->mSwapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(pCommandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mQuadGraphicsPipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(mInit->mSwapChainExtent.width);
  viewport.height = static_cast<float>(mInit->mSwapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(pCommandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent = mInit->mSwapChainExtent;
  vkCmdSetScissor(pCommandBuffer, 0, 1, &scissor);

  VkBuffer vertexBuffers[] = {mCubeVertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, vertexBuffers, offsets);

  // vkCmdBindIndexBuffer(pCommandBuffer, mQuadIndexBuffer, 0,
  //                      VK_INDEX_TYPE_UINT16);

  vkCmdBindDescriptorSets(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mQuadPipelineLayout, 0, 1, &mDescSets[mCurrentFrame],
                          0, nullptr);

  // vkCmdDrawIndexed(pCommandBuffer, mNumQuadIndicesToDraw, 1, 0, 0, 0);

  vkCmdBindIndexBuffer(pCommandBuffer, mCubeIndexBuffer, 0,
                       VK_INDEX_TYPE_UINT16);

  vkCmdDrawIndexed(pCommandBuffer, mNumCubeIndicesToDraw, 1, 0, 0, 0);

  vkCmdEndRenderPass(pCommandBuffer);

  if (vkEndCommandBuffer(pCommandBuffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to record command buffer!");
}

void VulkanRenderData::recordCircCommandBuffer(VkCommandBuffer pCommandBuffer,
                                               uint32_t pImageIndex) {
  assert(mInit);
  VkCommandBufferBeginInfo beginInfo{};
  Utils::zeroInitializeStruct(beginInfo);
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(pCommandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  Utils::zeroInitializeStruct(renderPassInfo);
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = mRenderPassFinal;
  renderPassInfo.framebuffer = mFramebuffers[pImageIndex];
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = mInit->mSwapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(pCommandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mCirclePipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(mInit->mSwapChainExtent.width);
  viewport.height = static_cast<float>(mInit->mSwapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(pCommandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent = mInit->mSwapChainExtent;
  vkCmdSetScissor(pCommandBuffer, 0, 1, &scissor);

  VkBuffer vertexBuffers[] = {mCircVertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(pCommandBuffer, /*mCircleIndexBuffer*/ mQuadIndexBuffer,
                       0, VK_INDEX_TYPE_UINT16);

  vkCmdBindDescriptorSets(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mCirclePipelineLayout, 0, 1,
                          &mDescSets[mCurrentFrame], 0, nullptr);

  vkCmdDrawIndexed(pCommandBuffer, mNumCirclesIndicesToDraw, 1, 0, 0, 0);

  vkCmdEndRenderPass(pCommandBuffer);

  if (vkEndCommandBuffer(pCommandBuffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to record command buffer!");
}

void VulkanRenderData::recordImGuiCommandBuffer(VkCommandBuffer pCommandBuffer,
                                                uint32_t pImageIndex) {
  assert(mInit);
  VkCommandBufferBeginInfo beginInfo{};
  Utils::zeroInitializeStruct(beginInfo);
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(pCommandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  Utils::zeroInitializeStruct(renderPassInfo);
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = mRenderPassFinal;
  renderPassInfo.framebuffer = mFramebuffers[pImageIndex];
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = mInit->mSwapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(pCommandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pCommandBuffer);

  vkCmdEndRenderPass(pCommandBuffer);

  if (vkEndCommandBuffer(pCommandBuffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to record command buffer!");
}
void VulkanRenderData::createSyncObjects() {
  assert(mInit);
  mImgAvailSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mRndrFinSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  Utils::zeroInitializeStruct(semaphoreInfo);
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  Utils::zeroInitializeStruct(fenceInfo);
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(mInit->mLogicalDevice, &semaphoreInfo, nullptr,
                          &mImgAvailSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(mInit->mLogicalDevice, &semaphoreInfo, nullptr,
                          &mRndrFinSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(mInit->mLogicalDevice, &fenceInfo, nullptr,
                      &mInFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create semaphores!");
    }
  }
}

void VulkanRenderData::prepareFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void VulkanRenderData::drawFrame(bool pFrameBufferResized) {
  assert(mInit);
  vkWaitForFences(mInit->mLogicalDevice, 1, &mInFlightFences[mCurrentFrame],
                  VK_TRUE, UINT64_MAX);
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      mInit->mLogicalDevice, mInit->mSwapChain, UINT64_MAX,
      mImgAvailSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Failed to acquire swap chain image!");
  }

  vkResetFences(mInit->mLogicalDevice, 1, &mInFlightFences[mCurrentFrame]);
  vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
  vkResetCommandBuffer(mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT],
                       0);
  vkResetCommandBuffer(
      mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT * 2], 0);

  ImGui::Render();
  updateUniformBuffer(mCurrentFrame);
  recordQuadCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);
  recordCircCommandBuffer(mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT],
                          imageIndex);
  recordImGuiCommandBuffer(
      mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT * 2], imageIndex);

  VkSubmitInfo submitInfo{};
  Utils::zeroInitializeStruct(submitInfo);
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {mImgAvailSemaphores[mCurrentFrame]};
  VkPipelineStageFlags waitStage[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStage;
  submitInfo.commandBufferCount = 3;
  VkCommandBuffer commandBuffersUsed[] = {
      mCommandBuffers[mCurrentFrame],
      mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT],
      mCommandBuffers[mCurrentFrame + MAX_FRAMES_IN_FLIGHT * 2]};
  submitInfo.pCommandBuffers =
      commandBuffersUsed; //&mCommandBuffers[mCurrentFrame];

  VkSemaphore signalSemaphores[] = {mRndrFinSemaphores[mCurrentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo,
                    mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
    throw std::runtime_error("Failed to submit draw command buffer!");

  VkPresentInfoKHR presentInfo{};
  Utils::zeroInitializeStruct(presentInfo);
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {mInit->mSwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      pFrameBufferResized) {
    recreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image!");
  }

  mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // either 0 or 1
}

void VulkanRenderData::updateEntityPos(uint32_t pEntityID,
                                       glm::vec3 pCurrentPos) {
  // this needs to take the difference between the current vertex/quad position
  // and the one saved in the vertex array, and store that value in the mUBOdata
  // buffer
  //
  // this is bad and needs to change
  mUbo.worldPos[pEntityID] = pCurrentPos - mQuadVertices[pEntityID * 4].pos;
}

void VulkanRenderData::updateUniformBuffer(uint32_t pCurrentImage) {
  // copy individual positions to the uniform buffer buffer
  // copy the entier ubo to the GPU
  UBO stagedUBO = mUbo;
  // have to stage it because mUbo is being written to on a different thread
  // potentially while vulkan would want to be reading to it
  memcpy(mUniformBuffersMapped[pCurrentImage], &stagedUBO, sizeof(UBO));
}

void VulkanRenderData::initNewQuad() {
  vkDeviceWaitIdle(mInit->mLogicalDevice); // this is terrible
  vkFreeCommandBuffers(mInit->mLogicalDevice, mCommandPool,
                       mCommandBuffers.size() / 3, mCommandBuffers.data());
  vkDestroyBuffer(mInit->mLogicalDevice, mQuadVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mQuadVertexBufferMem, nullptr);

  createVertexBuffer(mQuadVertices, mQuadVertexBuffer, mQuadVertexBufferMem);
  createCommandBuffers(0, mCommandBuffers.size() / 3);
}

void VulkanRenderData::initNewCirc() {
  vkDeviceWaitIdle(mInit->mLogicalDevice); // this is terrible
  vkFreeCommandBuffers(mInit->mLogicalDevice, mCommandPool,
                       mCommandBuffers.size() / 3,
                       mCommandBuffers.data() + mCommandBuffers.size() / 3);
  vkDestroyBuffer(mInit->mLogicalDevice, mCircVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mCircVertexBufferMem, nullptr);

  createVertexBuffer(mCircleVertices, mCircVertexBuffer, mCircVertexBufferMem);
  createCommandBuffers(mCommandBuffers.size() / 3,
                       mCommandBuffers.size() + mCommandBuffers.size() / 3);
}

void VulkanRenderData::initNewCube() {
  vkDeviceWaitIdle(mInit->mLogicalDevice); // this is terrible
  vkFreeCommandBuffers(mInit->mLogicalDevice, mCommandPool,
                       mCommandBuffers.size() / 3, mCommandBuffers.data());
  vkDestroyBuffer(mInit->mLogicalDevice, mCubeVertexBuffer, nullptr);
  vkFreeMemory(mInit->mLogicalDevice, mCubeVertexBufferMem, nullptr);

  createVertexBuffer(mCubeVertices, mCubeVertexBuffer, mCubeVertexBufferMem);
  createCommandBuffers(0, mCommandBuffers.size() / 3);
}

void VulkanRenderData::drawIndexed(uint32_t pNumQuadsToDraw /*=0*/,
                                   uint32_t pNumCircsToDraw,
                                   uint32_t pNumCubesToDraw) {
  mNumQuadIndicesToDraw = pNumQuadsToDraw * 6; // 6 indicies per quad
  mNumCirclesIndicesToDraw = pNumCircsToDraw * 6;
  mNumCubeIndicesToDraw = pNumCubesToDraw * 6 * 6;
}

void VulkanRenderData::clearVertices() {
  mQuadVertices.clear();
  mCubeVertices.clear();
  mCircleVertices.clear();
  mQuadVertices.resize(MAX_VERTEX_COUNT);
  mCubeVertices.resize(MAX_CUBE_VTX_COUNT);
  mCircleVertices.resize(MAX_VERTEX_COUNT);
}

void VulkanRenderData::recreateSwapchain() {
  // NOTE:This may cause issues because the swapchain is being destroyed and
  // recreated before these are being destroyed at all
  //
  mInit->updateWindowDimensions();
  for (auto framebuffer : mFramebuffers) {
    vkDestroyFramebuffer(mInit->mLogicalDevice, framebuffer, nullptr);
  }

  for (auto imageView : mSwapChainImageViews) {
    vkDestroyImageView(mInit->mLogicalDevice, imageView, nullptr);
  }
  getSwapChainImages();
  createImageViews();
  createFramebuffers();
}

void VulkanRenderData::initImGUI() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(mInit->GetWindowHandle(), true);
  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = mInit->mInstance;
  initInfo.PhysicalDevice = mInit->mPhysicalDevice;
  initInfo.Device = mInit->mLogicalDevice;
  initInfo.QueueFamily = mInit->mQFIndices.graphicsFamily.value();
  initInfo.Queue = mGraphicsQueue;
  // initInfo.PipelineCache = mQuadGraphicsPipeline;
  initInfo.DescriptorPool = mDescPool;
  initInfo.RenderPass = mRenderPassInit;
  initInfo.Subpass = 0;
  initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
  initInfo.ImageCount = mSwapChainImages.size();
  initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  initInfo.Allocator = nullptr;
  initInfo.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&initInfo);
}
