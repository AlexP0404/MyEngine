#ifndef ENGINE_SDL_HPP
#define ENGINE_SDL_HPP

#include <memory>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>

#include "../engine.hpp"
#include "textureSDL.hpp"

struct FontDeleter {
  void operator()(TTF_Font *font) const {
    TTF_CloseFont(font);
    font = nullptr;
  }
};

class EngineSDL : public Engine {

public:
  EngineSDL();
  ~EngineSDL() override;

  bool init() override;
  void shutdown() override;
  bool loadMedia() override;
  bool shouldQuit() override;

  bool setTextureCoorCentered(const std::string &textureName, int x,
                              int y) override;
  bool setTextureCoor(const std::string &textureName, int x, int y) override;

  bool setTextTexture(const std::string &textureName,
                      const std::string &fontName,
                      const std::string &text) override;

  bool createTextureFromFile(const std::string &textureName,
                             const fs::path &fileName) override;
  void renderTextures() override;
  void renderScreen() override;
  void clearScreen() override;
  void eraseTextures(const std::vector<std::string> &texturesToErase) override;
  void eraseTexture(const std::string &textureName) override;
  void drawNet() override;
  void drawDot(int dotX, int dotY, int dotRadius) override;
  void drawPaddles(int p1X, int p1Y, int p2X, int p2Y) override;
  void playBounce() override;

private:
  SDL_Window *gameWindow;
  SDL_Renderer *gameRenderer;
  SDL_Surface *gameIcon;
  SDL_Color textColor;
  SDL_Rect p1Rect;
  SDL_Rect p2Rect;
  SDL_Rect dotRect;

  Mix_Chunk *bounce;
  std::unordered_map<std::string, std::unique_ptr<TextureSDL>>
      textures; // map all textures on hash table
  std::unordered_map<std::string, TTF_Font *> fonts;
};

#endif
