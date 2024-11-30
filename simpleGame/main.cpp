#include <Timer.hpp>
#include <engine.hpp>
#include <engineVLK.hpp>

#include "imgui.h"

int main() {
  EngineVLK gameEngine;
  gameEngine.setScreenSize(640, 480);
  gameEngine.setTitle("Test Project");
  gameEngine.init();

  Timer loopTimer;
  bool shouldQuit = false;

  while (gameEngine.shouldQuit() == false) {
    // while (loopTimer.ElapsedMillis() < 10.f) // render every 10 ms
    //   ;
    gameEngine.clearScreen();
    // bool isDemoOpen = true;
    // ImGui::ShowDemoWindow(&isDemoOpen);
    {
      ImGui::Begin("FPS Counter", nullptr, ImGuiWindowFlags_NoResize);
      ImGui::Text("Average FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::End();
    }

    loopTimer.Reset();
    // gameEngine.drawDot(300, 200, 100);
    gameEngine.renderScreen();
  }
  return 0;
}
