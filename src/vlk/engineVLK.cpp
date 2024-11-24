#include "engineVLK.hpp"
#include "renderer.hpp"
#include <memory>

// allows things like the input class to access the current application instance
// while avoiding creating a whole new engine object
static EngineVLK *s_Instance = nullptr;
EngineVLK &EngineVLK::Get() { return *s_Instance; }

EngineVLK::EngineVLK() {
  s_Instance = this; // set the current application instance
  mNumEntities = 0;
}

EngineVLK::~EngineVLK() {
  s_Instance = nullptr; // instance is no longer valid
}

bool EngineVLK::init() {
  mpWindow =
      std::make_shared<WindowVLK>(m_ScreenWidth, m_ScreenHeight, m_GameTitle);
  mpRenderer = std::make_shared<Renderer>(mpWindow, m_GameTitle);
  return mpWindow != nullptr && mpRenderer != nullptr;
}

void EngineVLK::shutdown() {
  mpRenderer->devWaitIdle(); // THIS HAS TO BE CALLED WHEN GAME ENDS
}

bool EngineVLK::loadMedia() { return true; }

bool EngineVLK::shouldQuit() {
  if (mpWindow->windowShouldClose()) {
    mpRenderer->devWaitIdle();
    return true;
  }
  return false;
}

bool EngineVLK::setTextureCoorCentered(const std::string &textureName, int x,
                                       int y) {
  return true;
}

bool EngineVLK::setTextureCoor(const std::string &textureName, int x, int y) {
  return true;
}

bool EngineVLK::setTextTexture(const std::string &textureName,
                               const std::string &fontName,
                               const std::string &text) {
  return true;
}

bool EngineVLK::createTextureFromFile(const std::string &textureName,
                                      const fs::path &fileName) {
  return true;
}

void EngineVLK::renderTextures() {}

void EngineVLK::renderScreen() {
  mpWindow->pollEvents();
  if (mpWindow->mFrameBufferResized) {
    // if the window was resized/minimized, tell the renderer to resize
    // swapchain stuff, then once the renderer adjusts, toggle it back off so
    // the renderer doesnt waste time recreating swapchain stuff
    //
    // sends true first
    mpRenderer->setFrameBufferResized(mpWindow->mFrameBufferResized);
    mpRenderer->renderScreen();
    // then false
    mpRenderer->setFrameBufferResized(mpWindow->mFrameBufferResized = false);
  } else {
    // window not resized
    mpRenderer->renderScreen();
  }
  mNumEntities = 0;
}

void EngineVLK::clearScreen() { mpRenderer->BeginBatch(); }

void EngineVLK::eraseTextures(const std::vector<std::string> &texturesToErase) {
}

void EngineVLK::eraseTexture(const std::string &textureName) {}

void EngineVLK::drawNet() {
  const float netRadius = 1.0f;
  for (int i = 0; i < m_ScreenHeight; i += 4) {
    mpRenderer->DrawCircle({m_ScreenWidth / 2, i}, netRadius, glm::vec3(1.0f),
                           mNumEntities++);
  }
}

void EngineVLK::drawDot(int dotX, int dotY, int dotRadius) {
  /* mpRenderer->DrawQuad({dotX, dotY}, {dotRadius * 2, dotRadius * 2}, */
  /*                      glm::vec3(1.0f), mNumEntities++); */
  mpRenderer->DrawCircle({dotX, dotY}, static_cast<float>(dotRadius),
                         glm::vec3(1.0f), mNumEntities++);
}

void EngineVLK::drawPaddles(int p1X, int p1Y, int p2X, int p2Y) {
  // need to format the position and size based on (0,0) being the center of the
  // screen and (1,1) being bottom right etc
  //
  // gonna increment number of entities for each draw call then reset the count
  // back to 0 for each render so the ID is the same for each entity for each
  // loop as long as the loop doesn't change (may refactor later to be more
  // map-esque)
  mpRenderer->DrawQuad({p1X, p1Y}, {m_PaddleWidth, m_PaddleHeight},
                       {1.0f, 1.0f, 1.0f}, mNumEntities++);
  mpRenderer->DrawQuad({p2X, p2Y}, {m_PaddleWidth, m_PaddleHeight},
                       {1.0f, 1.0f, 1.0f}, mNumEntities++);
  /* mpRenderer->Flush(); */
}

void EngineVLK::playBounce() {}

std::shared_ptr<Renderer> EngineVLK::PFN_GetRenderer() { return mpRenderer; }
std::shared_ptr<WindowVLK> EngineVLK::PFN_GetWindowVLK() { return mpWindow; }
