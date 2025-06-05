#include "tile.hpp"

Tile::Tile(SDL_Texture* tex, int gridX, int gridY, int tileSize)
    : texture(tex) {
    
    // Optional 2.5D isometric tweak: offset Y based on gridX
    int x = gridX * tileSize + 100;
    int y = gridY * tileSize - gridX * 8 + 50;  // fake depth by shifting

    dstRect = { x, y, tileSize, tileSize };
}

void Tile::render(SDL_Renderer* renderer, int scrollX, int scrollY) {
    SDL_Rect screenRect = dstRect;
    screenRect.x -= scrollX;
    screenRect.y -= scrollY;
    SDL_RenderCopy(renderer, texture, nullptr, &screenRect);
}
