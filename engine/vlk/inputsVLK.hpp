#ifndef INPUTS_VLK_HPP
#define INPUTS_VLK_HPP

#include "keyCodes.hpp"
#include <initializer_list>

class GLFWwindow;

class inputVLK {
  GLFWwindow *m_windowHandle = nullptr;
  static inline float m_currMouseScroll = 0.0f;
  static inline double m_xPos = 0.0;
  static inline double m_yPos = 0.0;
  static inline bool m_mouseEventHappened = false;

public:
  inputVLK(GLFWwindow *windowHandle);
  ~inputVLK();

  bool IsKeyDown(KeyCode keycode) const;
  bool AreTheseKeyDown(const std::initializer_list<KeyCode> &keys) const;
  bool MouseEvent() const { return m_mouseEventHappened; }
  float MouseScroll() const { return m_currMouseScroll; }
  float MouseX() const { return m_xPos; }
  float MouseY() const { return m_yPos; }

  void resetMouseVals();

  static void scrollCallback(GLFWwindow *, double, double yoffset);
  static void mouseCallback(GLFWwindow *, double x, double y);
};

#endif // !INPUTS_VLK_HPP
