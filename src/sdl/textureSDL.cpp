#include "textureSDL.hpp"

TextureSDL::TextureSDL() {
  mTexture = NULL;
  mWidth = 0;
  mHeight = 0;
  mIsText = false;
}

TextureSDL::TextureSDL(SDL_Renderer &renderer, int xCoor, int yCoor,
                       bool isText)
    : gameRenderer(&renderer), xCoor(xCoor), yCoor(yCoor), mIsText(isText) {
  mTexture = NULL;
  mWidth = 0;
  mHeight = 0;
}

TextureSDL::~TextureSDL() { free(); }

void TextureSDL::setRenderer(SDL_Renderer &renderer) {
  this->gameRenderer = &renderer;
}

bool TextureSDL::loadFromFile(std::string path) {
  free();

  SDL_Texture *newTexture = NULL;

  SDL_Surface *loadedSurface = IMG_Load(path.c_str());
  if (loadedSurface == NULL) {
    printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(),
           IMG_GetError());
  } else {
    SDL_SetColorKey(loadedSurface, SDL_TRUE,
                    SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
    newTexture = SDL_CreateTextureFromSurface(gameRenderer, loadedSurface);
    if (newTexture == NULL) {
      printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(),
             SDL_GetError());
    } else {
      mWidth = loadedSurface->w;
      mHeight = loadedSurface->h;
    }
    SDL_FreeSurface(loadedSurface);
  }
  mTexture = newTexture;
  return mTexture != NULL;
}

bool TextureSDL::loadFromRenderedText(std::string textureText,
                                      SDL_Color textColor, TTF_Font *gFont) {
  // Get rid of preexisting texture
  free();
  // Render text surface
  SDL_Surface *textSurface =
      TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
  if (textSurface == NULL) {
    printf("Unable to render text surface! SDL_ttf Error: %s\n",
           TTF_GetError());
  } else {
    // Create texture from surface pixels
    mTexture = SDL_CreateTextureFromSurface(gameRenderer, textSurface);
    if (mTexture == NULL) {
      printf("Unable to create texture from rendered text! SDL Error: %s\n",
             SDL_GetError());
    } else {
      // Get image dimensions
      mWidth = textSurface->w;
      mHeight = textSurface->h;
    }

    // Get rid of old surface
    SDL_FreeSurface(textSurface);
  }

  return mTexture != NULL;
}

void TextureSDL::free() {
  // Free texture if it exists
  if (mTexture != NULL) {
    SDL_DestroyTexture(mTexture);
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
  }
}

void TextureSDL::setColor(Uint8 red, Uint8 green, Uint8 blue) {
  // Modulate texture rgb
  SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void TextureSDL::setBlendMode(SDL_BlendMode blending) {
  // Set blending function
  SDL_SetTextureBlendMode(mTexture, blending);
}

void TextureSDL::setAlpha(Uint8 alpha) {
  // Modulate texture alpha
  SDL_SetTextureAlphaMod(mTexture, alpha);
}

void TextureSDL::render(SDL_Rect *clip, double angle, SDL_Point *center,
                        SDL_RendererFlip flip) {
  // Set rendering space and render to screen
  SDL_Rect renderQuad = {xCoor, yCoor, mWidth, mHeight};

  // Set clip rendering dimensions
  if (clip != NULL) {
    renderQuad.w = clip->w;
    renderQuad.h = clip->h;
  }

  // Render to screen
  SDL_RenderCopyEx(gameRenderer, mTexture, clip, &renderQuad, angle, center,
                   flip);
}

int TextureSDL::getWidth() { return mWidth; }

int TextureSDL::getHeight() { return mHeight; }

int TextureSDL::getxCoor() { return xCoor; }

int TextureSDL::getyCoor() { return yCoor; }

void TextureSDL::setxCoor(int x) { xCoor = x; }

void TextureSDL::setyCoor(int y) { yCoor = y; }

void TextureSDL::setCoors(int x, int y) { xCoor = x, yCoor = y; }
