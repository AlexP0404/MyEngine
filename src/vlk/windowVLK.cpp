#include "windowVLK.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept> //throw runtime_error

WindowVLK::WindowVLK(const std::string_view &pWindowTitle)
    : mWindowTitle(pWindowTitle) {
  mFrameBufferResized = false;
  initWindow(1280, 720); // default resolution
}

WindowVLK::WindowVLK(int pWindowWidth, int pWindowHeight,
                     const std::string_view &pWindowTitle)
    : mWindowTitle(pWindowTitle) {
  mFrameBufferResized = false; // init variables
  initWindow(pWindowWidth, pWindowHeight);
}

void WindowVLK::frameBufferResizeCallback(GLFWwindow *pWindow, int pWidth,
                                          int pHeight) {
  auto app = reinterpret_cast<WindowVLK *>(glfwGetWindowUserPointer(pWindow));
  app->mFrameBufferResized = true;
}

WindowVLK::~WindowVLK() {
  glfwDestroyWindow(mWindowHandle);
  glfwTerminate();
}

void WindowVLK::initWindow(int pWindowWidth, int pWindowHeight) {
  glfwInit(); // init GLFW
  glfwWindowHint(GLFW_CLIENT_API,
                 GLFW_NO_API); // tell glfw using Vulkan not OpenGL
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // not resizable rn

  mWindowHandle = glfwCreateWindow(pWindowWidth, pWindowHeight,
                                   mWindowTitle.data(), nullptr,
                                   nullptr); // create window handle
  if (mWindowHandle == nullptr)
    throw std::runtime_error("Failed to open GLFW window!");
  glfwSetWindowUserPointer(mWindowHandle, this); // move the cursor to game
  glfwSetFramebufferSizeCallback(
      mWindowHandle,
      frameBufferResizeCallback); //(doesn't do anything rn because not
  // resizable) called when the window resizes
  // so the swapchain can be rebuilt
}

void WindowVLK::pollEvents() { glfwPollEvents(); }

bool WindowVLK::windowShouldClose() {
  return glfwWindowShouldClose(mWindowHandle);
}

GLFWwindow *WindowVLK::getWindowHandle() { return mWindowHandle; }
