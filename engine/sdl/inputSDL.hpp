#ifndef INPUT_SDL_HPP
#define INPUT_SDL_HPP

#include "keyCodes.hpp"

class inputSDL {
private:
  inline uint8_t convertKeycode(KeyCode keycode);
  const uint8_t *keyState = nullptr;

public:
  void updateKeyStates();
  bool IsKeyDown(KeyCode keycode);
};

#endif
