#include "renderer.hpp"
#include "circleVertex.hpp"
#include "vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>

static glm::vec2 sWinDimensions;

Renderer::Renderer(std::shared_ptr<WindowVLK> pWindow,
                   std::string_view pWindowTitle)
    : mWindowVLK(pWindow) {
  mVLKInit = std::make_shared<VulkanInit>();
  mVLKInit->Init(pWindow->getWindowHandle(), pWindowTitle);
  sWinDimensions = {mVLKInit->mSwapChainExtent.width,
                    mVLKInit->mSwapChainExtent.height};

  mVLKData.initRenderData(mVLKInit);
  mFrameBufferResized = false;
  mNewCircleAdded = false;
  mNewQuadAdded = false;
  mNumQuadsDrawn = 0;
  mNumCubesDrawn = 0;
  mNumCirclesDrawn = 0;
  mVLKData.clearVertices();
}
// this may not work if the device is already destroyed. it shouldnt tho?
Renderer::~Renderer() {}

void Renderer::setFrameBufferResized(bool pFrameBufferResized) {
  mFrameBufferResized = pFrameBufferResized;
}

void Renderer::devWaitIdle() { mVLKData.devWaitIdle(); }

const glm::vec3 Renderer::convertPosition(const glm::vec3 &pPosition) {
  // center around top left instead of window center
  glm::vec2 xyPos = {pPosition.x, pPosition.y};
  return {xyPos / sWinDimensions * 2.0f - glm::vec2(1.0f), pPosition.z};
  /* return ret / sWinDimensions; // put in range 0-1 */
}

const glm::vec2 Renderer::convertSize(const glm::vec2 &pSize) {
  return pSize / sWinDimensions * 2.0f;
}

void Renderer::DrawCircle(const glm::vec3 &pPosition, const float pRadius,
                          const glm::vec3 &pColor, const uint32_t pCircleID) {
  // convert circle center position and radius,
  // then pass to render data
  //
  // rn, simple heuristic to always draw circles after quads making easy lookup
  size_t firstVertexIndex = (pCircleID - mNumQuadsDrawn) * 4;
  if (mVLKData.mCircleVertices.at(firstVertexIndex).color == pColor) {
    mVLKData.updateEntityPos(pCircleID, convertPosition(pPosition));
    return;
  }
  if (mNumQuadsDrawn + mNumCirclesDrawn >= mVLKData.MAX_QUAD_COUNT) {
    Flush();
    BeginBatch();
  }

  auto circleQuad =
      CreateCircle(convertPosition(pPosition),
                   convertSize(glm::vec2(pRadius)).y * 2.0f, pColor, pCircleID);
  mVLKData.mCircleVertices.insert(mVLKData.mCircleVertices.begin() +
                                      firstVertexIndex,
                                  circleQuad.begin(), circleQuad.end());
  // add circle to the list
  mNewCircleAdded = true;
  mNumCirclesDrawn++;
}

const std::array<CircleVertex, 4>
Renderer::CreateCircle(const glm::vec3 &pPosition, const float pRadius,
                       const glm::vec3 &pColor, const uint32_t pCircleID) {
  CircleVertex v0;
  v0.worldPos = pPosition;
  v0.localPos = {QUAD_VERTEX_POS[0].x * 2.0f, QUAD_VERTEX_POS[0].y * 2.0f};
  v0.radius = pRadius;
  v0.color = pColor;
  v0.entityID = pCircleID;

  CircleVertex v1;
  v1.worldPos = {pPosition.x + pRadius, pPosition.y, 1.0};
  v1.localPos = {QUAD_VERTEX_POS[1].x * 2.0f, QUAD_VERTEX_POS[1].y * 2.0f};
  v1.radius = pRadius;
  v1.color = pColor;
  v1.entityID = pCircleID;

  CircleVertex v2;
  v2.worldPos = {pPosition.x + pRadius, pPosition.y + pRadius, 1.0};
  v2.localPos = {QUAD_VERTEX_POS[2].x * 2.0f, QUAD_VERTEX_POS[2].y * 2.0f};
  v2.radius = pRadius;
  v2.color = pColor;
  v2.entityID = pCircleID;

  CircleVertex v3;
  v3.worldPos = {pPosition.x, pPosition.y + pRadius, 1.0};
  v3.localPos = {QUAD_VERTEX_POS[3].x * 2.0f, QUAD_VERTEX_POS[3].y * 2.0f};
  v3.radius = pRadius;
  v3.color = pColor;
  v3.entityID = pCircleID;

  return {v0, v1, v2, v3};
}

const std::array<Vertex, 4> Renderer::CreateQuad(const glm::vec3 &pPosition,
                                                 const glm::vec2 &pSize,
                                                 const glm::vec3 &pColor,
                                                 const uint32_t pQuadID) {
  Vertex v0{.pos = pPosition, .entityID = pQuadID, .color = pColor};

  Vertex v1{.pos = {pPosition.x + pSize.x, pPosition.y, pPosition.z},
            .entityID = pQuadID,
            .color = pColor};

  Vertex v2{
      .pos = {pPosition.x + pSize.x, pPosition.y + pSize.y, pPosition.z},
      .entityID = pQuadID,
      .color = pColor,
  };

  Vertex v3{
      .pos = {pPosition.x, pPosition.y + pSize.y, pPosition.z},
      .entityID = pQuadID,
      .color = pColor,
  };

  return {v0, v1, v2, v3};
}

void Renderer::DrawQuad(const glm::vec3 &pPosition, const glm::vec2 &pSize,
                        const glm::vec3 &pColor, const uint32_t pQuadID) {
  /* DrawQuad({pPosition.x, pPosition.y, 0.0f}, pSize, pColor); */
  // if quad already exists, don't draw a new one, but update ubo position
  if (mVLKData.mQuadVertices.at(pQuadID * 4).color == pColor) {
    mVLKData.updateEntityPos(pQuadID, convertPosition(pPosition));
    return;
  }

  if (mNumQuadsDrawn + mNumCirclesDrawn >= mVLKData.MAX_QUAD_COUNT) {
    Flush();
    BeginBatch();
  }
  auto quad = CreateQuad(convertPosition(pPosition), convertSize(pSize), pColor,
                         pQuadID);
  // quadID = current quads drawn

  mVLKData.mQuadVertices.insert(mVLKData.mQuadVertices.begin() + (pQuadID * 4),
                                quad.begin(), quad.end());
  mNumQuadsDrawn++;
  mNewQuadAdded = true;
  // add quad vertices to the vertices vector
}

void Renderer::DrawCube(const glm::vec3 &pPosition, const glm::vec3 &pColor,
                        const float pRadius, const uint32_t pCubeID) {
  // generate 6 quad faces to make the cube
  std::array<Vertex, 8> cubeVertices;
  std::array<glm::vec3, 8> cubePos = {
      glm::vec3(-1, -1, 0.5) + pPosition,  // 0
      glm::vec3(1, -1, 0.5) + pPosition,   // 1
      glm::vec3{-1, 1, 0.5} + pPosition,   // 2
      glm::vec3{1, 1, 0.5} + pPosition,    // 3
      glm::vec3{-1, -1, -0.5} + pPosition, // 4
      glm::vec3{1, -1, -0.5} + pPosition,  // 5
      glm::vec3{-1, 1, -0.5} + pPosition,  // 6
      glm::vec3{1, 1, -0.5} + pPosition    // 7
  };
  for (auto &vert : cubePos) {
    vert.x *= pRadius;
    vert.y *= pRadius;
    vert.z *= pRadius;
  }

  for (int i = 0; i < 8; i++) {
    cubeVertices[i].pos = cubePos[i];
    cubeVertices[i].color = pColor;
    cubeVertices[i].entityID = pCubeID;
  }
  mVLKData.mCubeVertices.insert(mVLKData.mCubeVertices.begin(),
                                cubeVertices.begin(), cubeVertices.end());
  mNumCubesDrawn++;
  mNewCubeAdded = true;
}

void Renderer::renderScreen() {
  if (mNewCircleAdded) {
    mVLKData.initNewCirc();
    mNewCircleAdded = false;
  }
  if (mNewQuadAdded) {
    mVLKData.initNewQuad();
    mNewQuadAdded = false;
  }
  if (mNewCubeAdded) {
    mVLKData.initNewCube();
    mNewCubeAdded = false;
  }

  Flush();
  mVLKData.setViewMatrix(mCamera->GetViewMatrix());
  mVLKData.drawFrame(mFrameBufferResized); // should only need to resize once
  if (mFrameBufferResized)                 // update window dimensions
    sWinDimensions = {mVLKInit->mSwapChainExtent.width,
                      mVLKInit->mSwapChainExtent.height};
}

void Renderer::Flush() {
  mVLKData.drawIndexed(mNumQuadsDrawn, mNumCirclesDrawn, mNumCubesDrawn);
}
// TODO:: fix this naming
void Renderer::BeginBatch() {
  mVLKData.prepareFrame();
  // mNumQuadsDrawn = 0;
  // mNumCirclesDrawn = 0;
  // mNumCubesDrawn = 0;
  // mVLKData.clearVertices();
}
