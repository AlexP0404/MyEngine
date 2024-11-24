#ifndef WINDOW_VLK_HPP
#define WINDOW_VLK_HPP

#include <string_view>

struct GLFWwindow;

class WindowVLK {
public:
  explicit WindowVLK(const std::string_view &pWindowTitle);
  WindowVLK(int pWindowWidth, int pWindowHeight,
            const std::string_view &pWindowTitle);
  ~WindowVLK();

  void pollEvents();
  bool windowShouldClose();

  GLFWwindow *getWindowHandle();

  bool mFrameBufferResized; // used for swapchain recreation

private:
  void initWindow(int pWindowWidth, int pWindowHeight);
  static void frameBufferResizeCallback(GLFWwindow *pWindow, int pWidth,
                                        int pHeight);

private:
  std::string_view mWindowTitle;
  GLFWwindow *mWindowHandle;
};

#endif
