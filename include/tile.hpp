#pragma once
#include <SDL2/SDL.h>

class Tile {
public:
    Tile(SDL_Texture* texture, int gridX, int gridY, int tileSize = 32);

    void render(SDL_Renderer* renderer, int scrollX = 0, int scrollY = 0);

private:
    SDL_Texture* texture;
    SDL_Rect dstRect;
};
