#include "inputsVLK.hpp"

#include "engineVLK.hpp"

#include <GLFW/glfw3.h>

inputVLK::~inputVLK() { windowHandle = nullptr; }

void inputVLK::updateKeyStates() {
  if (windowHandle) // only need to get the handle once
    return;

  windowHandle = EngineVLK::Get().PFN_GetWindowVLK()->getWindowHandle();
}

bool inputVLK::IsKeyDown(KeyCode key) {
  if (!windowHandle) // somehow if window isn't create yet or already destroyed
    return false;

  int state = glfwGetKey(windowHandle, (int)key);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}
