#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "tile_instance.hpp"

class World {
public:
    World(SDL_Renderer* renderer);
    ~World();
    void render(int scrollX, int scrollY);

    
    float zoom = 1.0f;  // default: 100%

private:
    SDL_Renderer* renderer;

    SDL_Texture* grassTexture;
    SDL_Texture* waterTexture;
    SDL_Texture* rockTexture;

    std::vector<TileInstance> tiles;

    void generateWorld(int width, int height);
    SDL_Texture* loadTexture(const char* path);
    SDL_Texture* cliffTexture;
    int getHeightAt(int x, int y);

    void generateMountains(std::vector<std::vector<bool>>& featureMask,std::vector<std::vector<int>>& heightMap, int width, int height, int count);
    void generateValleys(std::vector<std::vector<bool>>& featureMask,std::vector<std::vector<int>>& heightMap, int width, int height, int count);

};
