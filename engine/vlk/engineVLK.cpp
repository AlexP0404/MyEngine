#include "engineVLK.hpp"
#include "Timer.hpp"
#include <memory>

EngineVLK::EngineVLK() { mNumEntities = 0; }

EngineVLK::~EngineVLK() { shutdown(); }

bool EngineVLK::init() {
  mpWindow =
      std::make_shared<WindowVLK>(m_ScreenWidth, m_ScreenHeight, m_GameTitle);
  mCamera = std::make_shared<Camera>();
  mCamera->MovementSpeed = 1.f;
  mInputs = std::make_shared<inputVLK>(mpWindow->getWindowHandle());

  mpRenderer = std::make_shared<Renderer>(mpWindow, m_GameTitle);
  mpRenderer->setCamera(mCamera);
  return mpWindow != nullptr && mpRenderer != nullptr;
}

void EngineVLK::shutdown() {
  mpRenderer->devWaitIdle(); // THIS HAS TO BE CALLED WHEN GAME ENDS
}

bool EngineVLK::loadMedia() { return true; }

bool EngineVLK::shouldQuit() {
  bool const userPressedQuit =
      mInputs->IsKeyDown(KeyCode::Escape) || mInputs->IsKeyDown(KeyCode::Q);
  if (mpWindow->windowShouldClose() || userPressedQuit == true) {
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

uint32_t EngineVLK::addMesh(const fs::path &meshPath,
                            std::string_view textureName, glm::vec3 position) {
  // TODO:: CREATE THE MESH CLASS THAT LOADS A 3D model (gets its vertices,
  // indices, normals etc)
  //  AND ITS ASSOCIATED TEXTURE IF DESIRED
  return mNumEntities++; // this will need to be adjusted ofc
}

void EngineVLK::renderTextures() {}

void EngineVLK::renderScreen() {
  static Timer updateTimer;
  mpWindow->pollEvents();
  if (mpWindow->mFrameBufferResized == false) {
    // window not resized
    mpRenderer->renderScreen();
  } else {
    // if the window was resized/minimized, tell the renderer to resize
    // swapchain stuff, then once the renderer adjusts, toggle it back off so
    // the renderer doesnt waste time recreating swapchain stuff
    //
    // sends true first
    mpRenderer->setFrameBufferResized(mpWindow->mFrameBufferResized);
    mpRenderer->renderScreen();
    // then false
    mpRenderer->setFrameBufferResized(mpWindow->mFrameBufferResized = false);
  }
  // mNumEntities = 0;
  Update(updateTimer.ElapsedMillis());
  updateTimer.Reset();
}

void EngineVLK::addCube() {
  mpRenderer->DrawCube(mCamera->Position, {1, 0, 0}, 100, mNumEntities++);
}

// will be used for more updates like movement of meshes
void EngineVLK::Update(float ts) { checkCameraUpdate(ts); }

void EngineVLK::checkCameraUpdate(float ts) {
  if (mInputs->AreTheseKeyDown({KeyCode::Down, KeyCode::S})) {
    mCamera->ProcessKeyboard(BACKWARD, ts);
  }
  if (mInputs->AreTheseKeyDown({KeyCode::Up, KeyCode::W})) {
    mCamera->ProcessKeyboard(FORWARD, ts);
  }
  if (mInputs->AreTheseKeyDown({KeyCode::Left, KeyCode::A})) {
    mCamera->ProcessKeyboard(LEFT, ts);
  }
  if (mInputs->AreTheseKeyDown({KeyCode::Right, KeyCode::D})) {
    mCamera->ProcessKeyboard(RIGHT, ts);
  }
  if (mInputs->MouseEvent() == true) {
    mCamera->ProcessMouseScroll(mInputs->MouseScroll());
    mCamera->ProcessMouseMovement(mInputs->MouseX(), mInputs->MouseY());
    mInputs->resetMouseVals();
  }
}

void EngineVLK::clearScreen() { mpRenderer->BeginBatch(); }

void EngineVLK::eraseTextures(const std::vector<std::string> &texturesToErase) {
}

void EngineVLK::eraseTexture(const std::string &textureName) {}
