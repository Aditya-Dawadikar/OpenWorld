#include "player.hpp"
#include <SDL2/SDL_image.h>
#include <stdexcept>


bool isMoving = false;

Player::Player(SDL_Renderer* renderer, const char* spriteSheetPath, int frameW, int frameH)
    : frameWidth(frameW), frameHeight(frameH),
      currentFrame(0), frameRow(0),
      lastFrameTime(0), frameDelayMs(100)  // 100ms = 10 FPS
{
    SDL_Surface* surface = IMG_Load(spriteSheetPath);
    if (!surface)
        throw std::runtime_error(std::string("Player sprite load failed: ") + IMG_GetError());

    spriteSheet = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void Player::render(SDL_Renderer* renderer, int screenX, int screenY) {
    SDL_Rect src = { currentFrame * frameWidth, frameRow * frameHeight, frameWidth, frameHeight };
    SDL_Rect dst = { screenX, screenY, frameWidth, frameHeight };
    SDL_RenderCopy(renderer, spriteSheet, &src, &dst);
}

void Player::setDirection(int dx, int dy) {
    if (dy < 0) frameRow = 3;       // up
    else if (dy > 0) frameRow = 0;  // down
    else if (dx < 0) frameRow = 1;  // left
    else if (dx > 0) frameRow = 2;  // right
}

void Player::setMoving(bool moving) {
    isMoving = moving;
}

void Player::update() {
    if (!isMoving) return;  // 👈 skip frame updates if not moving

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime >= frameDelayMs) {
        lastFrameTime = currentTime;
        currentFrame = (currentFrame + 1) % 6; // Assuming 6 frames per direction
    }
}
