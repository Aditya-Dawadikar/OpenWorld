#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "tile_instance.hpp"

class World {
public:
    World(SDL_Renderer* renderer, int width, int height);
    ~World();
    void render(int scrollX, int scrollY);

    
    float zoom = 1.0f;  // default: 100%

private:
    SDL_Renderer* renderer;

    SDL_Texture* grassTexture;
    SDL_Texture* waterTexture;
    SDL_Texture* rockTexture;
    SDL_Texture* bushTexture;
    SDL_Texture* dirtTexture;

    std::vector<TileInstance> tiles;

    int width, height;

    std::vector<std::vector<int>> heightMap;
    std::vector<std::vector<TileType>> typeMap;

    std::vector<std::vector<bool>> featureMask;
    std::vector<std::vector<bool>> valleySeed;
    std::vector<std::vector<bool>> mountainSeed;
    std::vector<std::vector<bool>> lakeSeed;

    void generateWorld();
    SDL_Texture* loadTexture(const char* path);
    SDL_Texture* cliffTexture;
    int getHeightAt(int x, int y);

    void generateMountains(int count, int spreadRadius, int minHeight, int maxHeight, int fallOffRange);
    void generateValleys(int count, int minDepth, int maxDepth);
    void generateBush(int density);
    void generateDirt(int density);

};
