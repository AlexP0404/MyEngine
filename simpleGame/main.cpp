#include <engine.hpp>
#include <engineVLK.hpp>

#include "imgui.h"

int main() {
  EngineVLK gameEngine;
  gameEngine.setScreenSize(640, 480);
  gameEngine.setTitle("Test Project");
  gameEngine.init();

  bool shouldQuit = false;

  while (gameEngine.shouldQuit() == false) {
    gameEngine.clearScreen();
    bool isDemoOpen = true;
    ImGui::ShowDemoWindow(&isDemoOpen);
    {
      ImGui::Begin("FPS Counter", nullptr);
      if (ImGui::Button("Add Cube"))
        gameEngine.addCube();
      const auto pos = gameEngine.GetCamera()->Position;
      const auto zoom = gameEngine.GetCamera()->Zoom;
      const auto pitch = gameEngine.GetCamera()->Pitch;
      const auto yaw = gameEngine.GetCamera()->Yaw;
      ImGui::Text("Cam Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
      ImGui::Text("Cam zoom: %.1f", zoom);
      ImGui::Text("Cam pitch: %.1f", pitch);
      ImGui::Text("Cam yaw: %.1f", yaw);
      ImGui::Text("Average FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // gameEngine.drawDot(300, 200, 100);
    gameEngine.renderScreen();
  }
  return 0;
}
