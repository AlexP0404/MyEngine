#include "inputsVLK.hpp"

#include <GLFW/glfw3.h>

inputVLK::inputVLK(GLFWwindow *windowHandle) : m_windowHandle(windowHandle) {}

inputVLK::~inputVLK() {}

bool inputVLK::IsKeyDown(KeyCode key) {
  if (!m_windowHandle) // somehow if window isn't create yet or already
                       // destroyed
    return false;

  int state = glfwGetKey(m_windowHandle, (int)key);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}
