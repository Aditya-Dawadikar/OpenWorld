#pragma once
#include <SDL2/SDL.h>

class Player {
public:
    Player(SDL_Renderer* renderer, const char* spriteSheetPath, int frameW, int frameH);

    void update();
    void render(SDL_Renderer* renderer, int screenX, int screenY);

    void setDirection(int dx, int dy); // call this on keypress

    void setMoving(bool moving);  // <-- add this


private:
    SDL_Texture* spriteSheet;
    int frameWidth, frameHeight;
    int currentFrame;
    int frameRow; // which direction row
    int tickCount;
    int ticksPerFrame;

    Uint32 lastFrameTime;
    Uint32 frameDelayMs;
};
