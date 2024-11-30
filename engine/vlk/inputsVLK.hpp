#ifndef INPUTS_VLK_HPP
#define INPUTS_VLK_HPP

#include "keyCodes.hpp"

struct GLFWwindow;

class inputVLK {
  GLFWwindow *m_windowHandle = nullptr;

public:
  inputVLK(GLFWwindow *windowHandle);
  ~inputVLK();
  bool IsKeyDown(KeyCode keycode);
};

#endif // !INPUTS_VLK_HPP
