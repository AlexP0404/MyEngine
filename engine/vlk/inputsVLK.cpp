#include "inputsVLK.hpp"

#include <GLFW/glfw3.h>

inputVLK::inputVLK(GLFWwindow *windowHandle) : m_windowHandle(windowHandle) {
  glfwSetScrollCallback(windowHandle, &inputVLK::scrollCallback);
  glfwSetCursorPosCallback(windowHandle, &inputVLK::mouseCallback);
}

inputVLK::~inputVLK() {}

bool inputVLK::IsKeyDown(KeyCode key) const {
  if (!m_windowHandle) // somehow if window isn't create yet or already
                       // destroyed
    return false;

  int state = glfwGetKey(m_windowHandle, (int)key);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool inputVLK::AreTheseKeyDown(
    const std::initializer_list<KeyCode> &keys) const {
  if (!m_windowHandle) {
    return false;
  }
  for (const auto &key : keys) {
    int state = glfwGetKey(m_windowHandle, (int)key);
    if (state == GLFW_PRESS || state == GLFW_REPEAT) {
      return true;
    }
  }
  return false;
}

void inputVLK::scrollCallback(GLFWwindow *, double, double yoffset) {
  m_mouseEventHappened = true;
  m_currMouseScroll = yoffset;
}
void inputVLK::mouseCallback(GLFWwindow *window, double x, double y) {
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) != GLFW_PRESS) {
    return;
  }
  m_xPos = x;
  m_yPos = y;
  m_mouseEventHappened = true;
}
void inputVLK::resetMouseVals() {
  m_xPos = 0;
  m_yPos = 0;
  m_currMouseScroll = 0;
  m_mouseEventHappened = false;
}
