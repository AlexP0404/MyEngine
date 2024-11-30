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

  // will need a 3d mesh builder
  uint32_t addMesh(const fs::path &meshPath, std::string_view textureName = "",
                   glm::vec3 position = {0, 0, 0});

private:
  std::shared_ptr<Renderer>
      mpRenderer; // render class that sets up vulkan and handles all internal
                  // structures and has some basic drawing primitives
  std::shared_ptr<WindowVLK> mpWindow; // simple glfw window class setup
  std::shared_ptr<inputVLK> mInputs;
  std::shared_ptr<Camera> mCamera;
  uint32_t mNumEntities;

  void Update(float ts);
  void checkCameraUpdate(float ts);
};

#endif
