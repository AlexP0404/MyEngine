#ifndef ENGINE_VLK_HPP
#define ENGINE_VLK_HPP

#include "engine.hpp"
#include "inputsVLK.hpp"
#include "renderer.hpp"
#include "windowVLK.hpp"
#include <memory>

class EngineVLK : public Engine {

public:
  EngineVLK();
  ~EngineVLK() override;

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

  std::shared_ptr<Renderer> PFN_GetRenderer();
  std::shared_ptr<WindowVLK> PFN_GetWindowVLK();

private:
  std::shared_ptr<Renderer>
      mpRenderer; // render class that sets up vulkan and handles all internal
                  // structures and has some basic drawing primitives
  std::shared_ptr<WindowVLK> mpWindow; // simple glfw window class setup
  std::shared_ptr<inputVLK> mInputs;
  uint32_t mNumEntities;
};

#endif
