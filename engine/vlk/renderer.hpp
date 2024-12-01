#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Camera.hpp"
#include "circleVertex.hpp"
#include "textureVLK.hpp"
#include "vertex.hpp"
#include "vulkanInit.hpp"
#include "vulkanRenderData.hpp"
#include "windowVLK.hpp"

struct GLFWwindow;

class Renderer {
public:
  // pass the windowVLK as a shared_ptr so the GLFWwindow doesnt destruct before
  // renderer
  Renderer(std::shared_ptr<WindowVLK> pWindow, std::string_view pWindowTitle);
  ~Renderer();
  Renderer(Renderer &r) = delete;
  Renderer(Renderer &&r) = delete;
  void setFrameBufferResized(bool pFrameBufferResized = false);
  void setCamera(std::shared_ptr<Camera> pCamera) { mCamera = pCamera; }

  // Vulkan funcs
  void devWaitIdle();
  void renderScreen();

  // TODO:: redo the whole entity id system because its not expandible
  /**
   * Adds the given shapes to the draw queue.
   */
  void DrawCircle(const glm::vec3 &pPosition, const float pRadius,
                  const glm::vec3 &pColor, const uint32_t pCircleID);
  void DrawQuad(const glm::vec3 &pPosition, const glm::vec2 &pSize,
                const glm::vec3 &pColor, const uint32_t pQuadID);
  void DrawCube(const glm::vec3 &pPosition, const glm::vec3 &pColor,
                const float pRadius, const uint32_t pCubeID);
  // flushes the draw queue to draw
  void Flush();
  void BeginBatch();

private:
  std::shared_ptr<VulkanInit> mVLKInit;
  std::shared_ptr<WindowVLK> mWindowVLK;
  std::shared_ptr<Camera> mCamera;
  VulkanRenderData mVLKData;
  bool mFrameBufferResized;
  bool mNewCircleAdded;
  bool mNewQuadAdded;
  bool mNewCubeAdded;
  uint32_t mNumQuadsDrawn;
  uint32_t mNumCirclesDrawn;
  uint32_t mNumCubesDrawn;
  static constexpr glm::vec4 QUAD_VERTEX_POS[4] = {{-0.5f, -0.5f, 0.0f, 1.0f},
                                                   {0.5f, -0.5f, 0.0f, 1.0f},
                                                   {0.5f, 0.5f, 0.0f, 1.0f},
                                                   {-0.5f, 0.5f, 0.0f, 1.0f}};

  std::unordered_map<std::string_view, TextureVLK> mTextures;

  static const glm::vec3 convertPosition(const glm::vec3 &pPosition);
  static const glm::vec2 convertSize(const glm::vec2 &pSize);
  static const std::array<Vertex, 4> CreateQuad(const glm::vec3 &pPosition,
                                                const glm::vec2 &pSize,
                                                const glm::vec3 &pColor,
                                                const uint32_t pQuadID);
  static const std::array<CircleVertex, 4>
  CreateCircle(const glm::vec3 &pPosition, const float pRadius,
               const glm::vec3 &pColor, const uint32_t pCircleID);

  void EndBatch();
};

#endif // !RENDERER_HPP
