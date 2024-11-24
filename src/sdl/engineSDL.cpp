#include "engineSDL.hpp"
#include <iostream>

using std::string;
using std::unique_ptr;
using std::vector;

EngineSDL::EngineSDL() {
  gameWindow = nullptr;
  gameRenderer = nullptr;
  bounce = nullptr;
  textColor = {0xFF, 0xFF, 0xFF, 0xFF};
  m_ScreenWidth = 1280;
  m_ScreenHeight = 960;
}

EngineSDL::~EngineSDL() {
  for (auto &[name, font] : fonts) {
    TTF_CloseFont(font);
    font = nullptr;
  }

  Mix_FreeChunk(bounce);
  bounce = nullptr;

  SDL_DestroyRenderer(gameRenderer);
  SDL_DestroyWindow(gameWindow);
  gameWindow = nullptr;
  gameRenderer = nullptr;

  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

bool EngineSDL::init() {
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    success = false;
  } else {
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
      printf("Warning: Linear texture filtering not enabled!");
    }
    gameWindow = SDL_CreateWindow(
        m_GameTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_ScreenWidth, m_ScreenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS);
    if (gameWindow == nullptr) {
      printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
      success = false;
    } else {

      gameRenderer = SDL_CreateRenderer(
          gameWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (gameRenderer == nullptr) {
        printf("Renderer could not be created! SDL Error: %s\n",
               SDL_GetError());
        success = false;
      } else {
        SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
          printf("SDL_image could not initialize! SDL_image Error: %s\n",
                 IMG_GetError());
          success = false;
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
          printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n",
                 Mix_GetError());
          success = false;
        }
        if (TTF_Init() == -1) {
          printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
                 TTF_GetError());
          success = false;
        }
      }
    }
  }
  return success;
}

void EngineSDL::shutdown() {}

bool EngineSDL::loadMedia() {
  gameIcon = IMG_Load("res/dot.bmp");
  SDL_SetWindowIcon(gameWindow, gameIcon);

  fonts["escFont"] = TTF_OpenFont("res/lazy.ttf", 15);
  if (fonts.at("escFont") == nullptr) {
    printf("failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
    return false;
  }
  textures["escapePrompt"] = unique_ptr<TextureSDL>(new TextureSDL(
      *gameRenderer, 0, 0, true)); // uses full parameter constructor
  if (!textures.at("escapePrompt")
           ->loadFromRenderedText("Press Escape To Quit", textColor,
                                  fonts.at("escFont"))) {
    printf("Failed to render text texture!\n");
    return false;
  }

  fonts["mainFont"] = TTF_OpenFont("res/lazy.ttf", 30);

  bounce = Mix_LoadWAV("res/bounce.wav");
  if (bounce == nullptr) {
    printf("Failed to load bounce sound effect! SDL_mixer Error: %s\n",
           Mix_GetError());
    return false;
  }

  return true;
}

bool EngineSDL::shouldQuit() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    if (e.type == SDL_QUIT) {
      return true;
    }
  }
  return false;
}
// need to call setTextTexture then call setTextureCoor in order for the coors
// to save

bool EngineSDL::setTextureCoor(const string &textureName, int x, int y) {
  if (textures.find(textureName) == textures.end()) {
    std::cout << "Failed to set texture coor: " << textureName << "\n";
    return false;
  }
  textures.at(textureName)->setCoors(x, y);
  return true;
}

bool EngineSDL::setTextureCoorCentered(const string &textureName, int x,
                                       int y) {
  return setTextureCoor(std::move(textureName),
                        x - textures.at(textureName)->getWidth() / 2,
                        y - textures.at(textureName)->getHeight() / 2);
}

bool EngineSDL::setTextTexture(const string &textureName,
                               const string &fontName, const string &text) {
  if (textures.find(textureName) == textures.end()) {
    textures[textureName] =
        unique_ptr<TextureSDL>(new TextureSDL(*gameRenderer, 0, 0, true));
  }

  if (fonts.find(fontName) == fonts.end()) {
    std::cout << "Invalid font given! Using default font!\n";
    return textures.at(textureName)
        ->loadFromRenderedText(text.c_str(), textColor, fonts.at(fontName));
  }

  return textures.at(textureName)
      ->loadFromRenderedText(text.c_str(), textColor, fonts.at(fontName));
}

bool EngineSDL::createTextureFromFile(const string &textureName,
                                      const std::filesystem::path &fileName) {
  if (std::filesystem::is_empty(fileName)) {
    std::cout << "ERROR:: Texture does not exist!\n";
    return false;
  }
  textures[textureName] =
      unique_ptr<TextureSDL>(new TextureSDL(*gameRenderer, 0, 0, false));

  return textures.at(textureName)->loadFromFile(fileName.string());
}

void EngineSDL::renderTextures() {
  SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(gameRenderer);

  for (auto &[name, txtr] : textures) {
    txtr->render();
  }
}

void EngineSDL::renderScreen() { SDL_RenderPresent(gameRenderer); }

void EngineSDL::clearScreen() {
  /* SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 0xFF); */
  SDL_RenderClear(gameRenderer);
}

void EngineSDL::eraseTextures(const vector<string> &texturesToErase) {
  for (auto &textureName : texturesToErase) {
    textures.erase(textureName);
  }
}

void EngineSDL::eraseTexture(const string &textureName) {
  textures.erase(textureName);
}

void EngineSDL::drawNet() {
  SDL_SetRenderDrawColor(gameRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
  for (int i = 0; i < m_ScreenHeight; i += 4) {
    SDL_RenderDrawPoint(gameRenderer, m_ScreenWidth / 2, i);
  }
}

void EngineSDL::drawPaddles(int p1X, int p1Y, int p2X, int p2Y) {
  p1Rect = {p1X, p1Y, m_PaddleWidth, m_PaddleHeight};
  // initialize the first paddle in the middle left of the screen
  p2Rect = {p2X, p2Y, m_PaddleWidth, m_PaddleHeight};
  SDL_RenderFillRect(gameRenderer, &p1Rect);
  SDL_RenderFillRect(gameRenderer, &p2Rect);
}

void EngineSDL::drawDot(int dotX, int dotY, int dotRadius) {
  // https://www.ferzkopp.net/Software/SDL2_gfx/Docs/html/_s_d_l2__gfx_primitives_8c_source.html#l01457
  // used SDL_gfx filledCircle but didn't want to include the whole library just
  // for the circle so I edited it to use it here
  short cx = 0;
  short cy = dotRadius;
  short ocx = (short)0xffff;
  short ocy = (short)0xffff;
  short df = 1 - dotRadius;
  short d_e = 3;
  short d_se = -2 * dotRadius + 5;
  short xpcx, xmcx, xpcy, xmcy;
  short ypcy, ymcy, ypcx, ymcx;

  do {
    xpcx = dotX + cx;
    xmcx = dotX - cx;
    xpcy = dotX + cy;
    xmcy = dotX - cy;
    if (ocy != cy) {
      if (cy > 0) {
        ypcy = dotY + cy;
        ymcy = dotY - cy;
        SDL_RenderDrawLine(gameRenderer, xmcx, ypcy, xpcx, ypcy);
        SDL_RenderDrawLine(gameRenderer, xmcx, ymcy, xpcx, ymcy);
      } else {
        SDL_RenderDrawLine(gameRenderer, xmcx, dotY, xpcx, dotY);
      }
      ocy = cy;
    }
    if (ocx != cx) {
      if (cx != cy) {
        if (cx > 0) {
          ypcx = dotY + cx;
          ymcx = dotY - cx;
          SDL_RenderDrawLine(gameRenderer, xmcy, ymcx, xpcy, ymcx);
          SDL_RenderDrawLine(gameRenderer, xmcy, ypcx, xpcy, ypcx);
        } else {
          SDL_RenderDrawLine(gameRenderer, xmcy, dotY, xpcy, dotY);
        }
      }
      ocx = cx;
    }
    if (df < 0) {
      df += d_e;
      d_e += 2;
      d_se += 2;
    } else {
      df += d_se;
      d_e += 2;
      d_se += 4;
      cy--;
    }
    cx++;
  } while (cx <= cy);
}

void EngineSDL::playBounce() { Mix_PlayChannel(-1, bounce, 0); }
