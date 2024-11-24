#ifndef INPUTS_VLK_HPP
#define INPUTS_VLK_HPP

#include "keyCodes.hpp"

struct GLFWwindow;

class inputVLK {
  GLFWwindow *windowHandle = nullptr;

public:
  ~inputVLK();
  void updateKeyStates();
  bool IsKeyDown(KeyCode keycode);
};

#endif // !INPUTS_VLK_HPP
