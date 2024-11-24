#include "inputSDL.hpp"

#include <SDL2/SDL.h>

void inputSDL::updateKeyStates() { keyState = SDL_GetKeyboardState(nullptr); }

bool inputSDL::IsKeyDown(KeyCode keycode) {
  uint8_t key = convertKeycode(keycode);
  return keyState[key];
}

uint8_t inputSDL::convertKeycode(KeyCode keycode) {
  switch (keycode) {
  case KeyCode::Space:
    return SDL_SCANCODE_SPACE;
  case KeyCode::W:
    return SDL_SCANCODE_W;
  case KeyCode::S:
    return SDL_SCANCODE_S;
  case KeyCode::Up:
    return SDL_SCANCODE_UP;
  case KeyCode::Down:
    return SDL_SCANCODE_DOWN;
  case KeyCode::Left:
    return SDL_SCANCODE_LEFT;
  case KeyCode::Right:
    return SDL_SCANCODE_RIGHT;
  case KeyCode::D1:
    return SDL_SCANCODE_1;
  case KeyCode::D2:
    return SDL_SCANCODE_2;
  case KeyCode::D3:
    return SDL_SCANCODE_3;
  case KeyCode::Enter:
    return SDL_SCANCODE_RETURN;

  default:
    return SDL_SCANCODE_ESCAPE;
    break;
  }
}
