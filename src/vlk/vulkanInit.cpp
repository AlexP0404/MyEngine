#include "vulkanInit.hpp"

#include "utils.hpp"

#include <algorithm> //clamp
#include <cstdint>   //uint32_t
#include <iostream>  //cerr
#include <limits>    //numeric_limits
#include <set>       //
#include <stdexcept> //runtime_error

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
const bool enabledValidationLayers = false;
#else
const bool enabledValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

/* VulkanInit::VulkanInit() {} */

VulkanInit::~VulkanInit() {
  if (enabledValidationLayers) {
    DestroyDebugUtilsMessengerEXT(mInstance, debugMessenger, nullptr);
  }
  vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
  vkDestroyDevice(mLogicalDevice, nullptr);
  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);
}

void VulkanInit::Init(GLFWwindow *pWindowHandle,
                      std::string_view pWindowTitle) {
  mWindowHandle = pWindowHandle;
  mWindowTitle = pWindowTitle;
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
}

void VulkanInit::createInstance() {
  if (enabledValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("Validation layers enabled, but not available!");
  }
  VkApplicationInfo appInfo{};
  Utils::zeroInitializeStruct(appInfo);
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = mWindowTitle.data();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Alex's Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  Utils::zeroInitializeStruct(createInfo);
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (enabledValidationLayers) {
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    // allows the debug info for instance creation and destruction
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
  }
  VkResult res = vkCreateInstance(&createInfo, nullptr, &mInstance);
  if (res == VK_ERROR_LAYER_NOT_PRESENT) {
    throw std::runtime_error("Validation layer(s) not found!");
  }
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create instance!");
  }
}

bool VulkanInit::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
  for (auto validationLayer : validationLayers) {
    for (const auto &layerProperties : availableLayers) {
      if (strcmp(validationLayer, layerProperties.layerName) == 0) {
        return true; // the validation layer was found
      }
    }
  }
  return false;
}

inline const std::vector<const char *> VulkanInit::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  // fix annoying validation layer issue
  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  extensions.push_back("VK_KHR_get_physical_device_properties2");

  if (enabledValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInit::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

const void VulkanInit::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {

  Utils::zeroInitializeStruct(createInfo);
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
}

void VulkanInit::setupDebugMessenger() {
  if (!enabledValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr,
                                   &debugMessenger) != VK_SUCCESS)
    throw std::runtime_error("Failed to set up debug messenger!");
}

void VulkanInit::createSurface() {
  if (glfwCreateWindowSurface(mInstance, mWindowHandle, nullptr, &mSurface) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface!");
  }
}

void VulkanInit::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

  if (deviceCount == 0)
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

  mPhysicalDevice = pickBestPhysicalDevice(devices);

  if (mPhysicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("Failed to find suitable GPU!");
}

VkPhysicalDevice VulkanInit::pickBestPhysicalDevice(
    const std::vector<VkPhysicalDevice> &pDevices) {
  // just looks for descrete gpu with some specifc features
  VkPhysicalDevice potentialPhysDevice = VK_NULL_HANDLE;
  for (auto dev : pDevices) {
    VkPhysicalDeviceProperties devProperties;
    VkPhysicalDeviceFeatures devFeatures;
    vkGetPhysicalDeviceProperties(dev, &devProperties);
    vkGetPhysicalDeviceFeatures(dev, &devFeatures);

    QueueFamilyIndicies indicies = findQueueFamilies(dev);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(dev);
    bool swapChainAdequate = !swapChainSupport.formats.empty() &&
                             !swapChainSupport.presentModes.empty();
    if (indicies.isComplete() && swapChainAdequate &&
        devProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        devFeatures.geometryShader)
      return dev;
    if (indicies.isComplete() && swapChainAdequate &&
        devFeatures.geometryShader)
      potentialPhysDevice = dev; // if not discrete GPU but capable enough
  }
  return potentialPhysDevice; // if didn't find discrete GPU
}

VulkanInit::QueueFamilyIndicies
VulkanInit::findQueueFamilies(VkPhysicalDevice pDevice) {
  QueueFamilyIndicies indicies;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indicies.graphicsFamily = i;
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, mSurface,
                                           &presentSupport);
      if (presentSupport) {
        indicies.presentFamily = i;
      }

      if (indicies.isComplete())
        break;
    }
    i++;
  }

  return indicies;
}

void VulkanInit::createLogicalDevice() {
  mQFIndices = findQueueFamilies(mPhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamiles = {mQFIndices.graphicsFamily.value(),
                                           mQFIndices.presentFamily.value()};
  float queuePriority = 1.0f;
  for (auto queueFam : uniqueQueueFamiles) {
    VkDeviceQueueCreateInfo queueCreateInfo;
    Utils::zeroInitializeStruct(queueCreateInfo);
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = mQFIndices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  VkPhysicalDeviceFeatures deviceFeatures{};
  Utils::zeroInitializeStruct(deviceFeatures);

  VkDeviceCreateInfo createInfo{};
  Utils::zeroInitializeStruct(createInfo);
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  if (enabledValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  const char *devExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  createInfo.enabledExtensionCount = 1;
  createInfo.ppEnabledExtensionNames = &devExtension;

  if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) !=
      VK_SUCCESS)
    throw std::runtime_error("Failed to create logical device!");
}

VulkanInit::SwapChainSupportDetails
VulkanInit::querySwapChainSupport(VkPhysicalDevice pDevice) {
  SwapChainSupportDetails details;
  // capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, mSurface,
                                            &details.capabilities);
  // formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, mSurface, &formatCount,
                                       nullptr);
  if (formatCount > 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, mSurface, &formatCount,
                                         details.formats.data());
  }
  // present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, mSurface,
                                            &presentModeCount, nullptr);
  if (presentModeCount > 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        pDevice, mSurface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR VulkanInit::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &format : availableFormats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR VulkanInit::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &pMode : availablePresentModes) {
    if (pMode ==
        VK_PRESENT_MODE_MAILBOX_KHR) { // uncapped fps with less tearing
      /* std::cout << "USING PRESENT MODE MAILBOX\n"; */
      return pMode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR; // similar to VSYNC
}

VkExtent2D
VulkanInit::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(mWindowHandle, &width, &height);

  VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height)};

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);
  return actualExtent;
}

void VulkanInit::createSwapChain() {
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(mPhysicalDevice);
  // set the format, present mode, and extent
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  // set the image count to at least one more than the minimum as to not wait on
  // the driver
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  // make sure the +1 didn't put the imageCount above max though
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  Utils::zeroInitializeStruct(createInfo);
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = mSurface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndicies[] = {mQFIndices.graphicsFamily.value(),
                                    mQFIndices.presentFamily.value()};
  if (mQFIndices.graphicsFamily != mQFIndices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndicies;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  // don't care about pixels that are obscured
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(mLogicalDevice, &createInfo, nullptr, &mSwapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create swap chain!");
  }

  mSwapChainImageFormat = surfaceFormat.format;
  mSwapChainExtent = extent;
}

void VulkanInit::updateWindowDimensions() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(mWindowHandle, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(mWindowHandle, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(mLogicalDevice);
  // destroy swapchain then recreate it with correct size
  vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);

  createSwapChain();
}
