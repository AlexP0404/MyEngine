#ifndef TEXTURE_SDL_HPP
#define TEXTURE_SDL_HPP
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>

// this is too specifc to each backend to interface properly

class TextureSDL {
public:
  TextureSDL();
  TextureSDL(SDL_Renderer &renderer, int xCoor, int yCoor, bool isText);
  ~TextureSDL();
  TextureSDL(TextureSDL &t) {
    t.mTexture = mTexture;
    t.gameRenderer = gameRenderer;
    t.mWidth = mWidth;
    t.mHeight = mHeight;
    t.xCoor = xCoor;
    t.yCoor = yCoor;
    t.mIsText = mIsText;
  }

  void setRenderer(SDL_Renderer &renderer);
  bool loadFromFile(std::string path);
  bool loadFromRenderedText(std::string texturedText, SDL_Color textColor,
                            TTF_Font *gFont);
  void free();
  void setColor(Uint8 red, Uint8 green, Uint8 blue);
  void setBlendMode(SDL_BlendMode blending);
  void setAlpha(Uint8 alpha);
  void render(SDL_Rect *clip = NULL, double angle = 0.0,
              SDL_Point *center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
  int getWidth();
  int getHeight();
  int getxCoor();
  int getyCoor();
  void setxCoor(int x);
  void setyCoor(int y);
  void setCoors(int x, int y);

private:
  SDL_Texture *mTexture;
  int mWidth;
  int mHeight;
  int xCoor;
  int yCoor;
  SDL_Renderer *gameRenderer;
  bool mIsText;
};

#endif // !TEXTURE_HPP
