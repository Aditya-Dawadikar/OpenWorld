#include "world.hpp"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <queue>
#include "globals.hpp"
#include <set>

World::World(SDL_Renderer* renderer)
    : renderer(renderer) {

    grassTexture = loadTexture("../assets/grass.png");
    waterTexture = loadTexture("../assets/water.png");
    rockTexture  = loadTexture("../assets/dirt.png");
    cliffTexture = loadTexture("../assets/rock.png");


    generateWorld(50, 50); // 50x50 tiles
}

World::~World() {
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(waterTexture);
    SDL_DestroyTexture(rockTexture);
}

SDL_Texture* World::loadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface)
        throw std::runtime_error(std::string("Failed to load texture: ") + IMG_GetError());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void World::generateWorld(int width, int height) {
    srand(SDL_GetTicks());
    tiles.clear();

    std::vector<std::vector<int>> heightMap(width, std::vector<int>(height));
    std::vector<std::vector<TileType>> typeMap(width, std::vector<TileType>(height, TILE_GRASS));

    std::vector<std::vector<bool>> featureMask(width, std::vector<bool>(height, false));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (rand()%100 > 75){
                heightMap[x][y] = (rand() % 3) - 1;  // yields -1, 0, or 1
            }else{
                heightMap[x][y] = 0;
            }
        }
    }

    generateMountains(featureMask, heightMap, width, height, 5);
    generateValleys(featureMask, heightMap, width, height, 5);
    

    // Generate tile instances
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tiles.push_back({
                typeMap[x][y],
                x,
                y,
                heightMap[x][y]
            });
        }
    }
}

void World::generateMountains(std::vector<std::vector<bool>>& featureMask, std::vector<std::vector<int>>& heightMap, int width, int height, int numPlateaus){
    const int plateauHeight = 6;
    const int plateauRadius = 10;    // flat area
    const int falloffRadius = 6;    // transition edge

    for (int i = 0; i < numPlateaus; ++i) {
        int centerX = rand() % width;
        int centerY = rand() % height;

        if (featureMask[centerX][centerY] == true){
            i--;
            continue;
        }

        int peakH = plateauHeight;

        std::set<std::pair<int, int>> coreTiles;
        std::queue<std::pair<int, int>> q;
        q.push({centerX, centerY});

        while (!q.empty() && coreTiles.size() < 20 + rand() % 15) {
            auto [x, y] = q.front();
            q.pop();

            if (x < 0 || y < 0 || x >= width || y >= height)
                continue;

            if (coreTiles.count({x, y}) == 0) {
                coreTiles.insert({x, y});
                heightMap[x][y] = peakH;

                featureMask[x][y] = true;

                // Add neighbors with random chance
                for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                    if (rand() % 100 < 60)
                        q.push({x + dx, y + dy});
                }
            }
        }

        std::queue<std::tuple<int, int, int>> falloffQ;

        // Seed: add all core edges
        for (const auto& [x, y] : coreTiles) {
            for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && ny >= 0 && nx < width && ny < height &&
                    !coreTiles.count({nx, ny})) {
                    falloffQ.push({nx, ny, peakH - 1});
                }
            }
        }

        // Expand with decaying height
        while (!falloffQ.empty()) {
            auto [x, y, h] = falloffQ.front();
            falloffQ.pop();

            if (x < 0 || y < 0 || x >= width || y >= height || h <= 0)
                continue;

            if (h > heightMap[x][y])
                heightMap[x][y] = h;
                featureMask[x][y] = true;

            for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                int nx = x + dx;
                int ny = y + dy;

                if (rand() % 100 < 80) {
                    falloffQ.push({nx, ny, h - 1});
                }
            }
        }

    }
}

void World::generateValleys(std::vector<std::vector<bool>>& featureMask,
                            std::vector<std::vector<int>>& heightMap,
                            int width, int height, int numValleys) {
    const int valleyDepth = 6;

    for (int i = 0; i < numValleys; ++i) {
        int centerX = rand() % width;
        int centerY = rand() % height;

        if (featureMask[centerX][centerY]) {
            i--; // retry
            continue;
        }

        int peakH = -valleyDepth;

        std::set<std::pair<int, int>> coreTiles;
        std::queue<std::pair<int, int>> q;
        q.push({centerX, centerY});

        while (!q.empty() && coreTiles.size() < 20 + rand() % 15) {
            auto [x, y] = q.front();
            q.pop();

            if (x < 0 || y < 0 || x >= width || y >= height)
                continue;

            if (coreTiles.count({x, y}) == 0) {
                coreTiles.insert({x, y});
                heightMap[x][y] = peakH;
                featureMask[x][y] = true;

                for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                    if (rand() % 100 < 60)
                        q.push({x + dx, y + dy});
                }
            }
        }

        std::queue<std::tuple<int, int, int>> falloffQ;

        for (const auto& [x, y] : coreTiles) {
            for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && ny >= 0 && nx < width && ny < height &&
                    !coreTiles.count({nx, ny})) {
                    falloffQ.push({nx, ny, peakH + 1});
                }
            }
        }

        while (!falloffQ.empty()) {
            auto [x, y, h] = falloffQ.front();
            falloffQ.pop();

            if (x < 0 || y < 0 || x >= width || y >= height || h >= 0)
                continue;

            if (h < heightMap[x][y]) {
                heightMap[x][y] = h;
                featureMask[x][y] = true;
            }

            for (auto [dx, dy] : { std::pair{-1,0}, {1,0}, {0,-1}, {0,1} }) {
                int nx = x + dx;
                int ny = y + dy;

                if (rand() % 100 < 80) {
                    falloffQ.push({nx, ny, h + 1});
                }
            }
        }
    }
}


// void World::render(int scrollX, int scrollY) {
//     const int tileWidth = 64;
//     const int tileHeight = 32;
//     const int verticalOverlap = tileHeight / 4; // 8
//     const int tilesPerHeight = 4;

//     std::sort(tiles.begin(), tiles.end(), [](const TileInstance& a, const TileInstance& b) {
//         return (a.gridX + a.gridY) < (b.gridX + b.gridY);
//     });

//     for (const auto& t : tiles) {
//         // SDL_Texture* topTex = nullptr;
//         // SDL_Texture* wallTex = cliffTexture;

//         // switch (t.type) {
//         //     case TILE_GRASS: topTex = grassTexture; break;
//         //     case TILE_WATER: topTex = waterTexture; break;
//         //     case TILE_ROCK:  topTex = rockTexture;  break;
//         // }

//         SDL_Texture* topTex = grassTexture;  // top tile always grass for now
//         SDL_Texture* wallTex = cliffTexture; // walls always rock


//         int isoX = (t.gridX - t.gridY) * (tileWidth / 2) - scrollX;
//         int isoY = (t.gridX + t.gridY) * (tileHeight / 2) - scrollY;
//         int topY = isoY - t.height * tilesPerHeight * verticalOverlap;

//         if (t.height > 0) {
//             // Stack walls downward (mountains)
//             for (int h = t.height * tilesPerHeight; h >= 1; --h) {
//                 SDL_Rect cliffDst = {
//                     isoX,
//                     topY + h * verticalOverlap,
//                     tileWidth,
//                     tileHeight
//                 };
//                 SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
//             }
//         } else if (t.height < 0) {
//             int totalSubTiles = -t.height * tilesPerHeight;

//             int baseDepth = -t.height;  // how deep the column starts
//             for (int s = 0; s <= totalSubTiles; ++s) {
//                 SDL_Rect cliffDst = {
//                     isoX,
//                     topY + s * verticalOverlap,
//                     tileWidth,
//                     tileHeight
//                 };

//                 int absoluteDepth = baseDepth * tilesPerHeight + s;

//                 // Start bright at shallow levels and fade rapidly
//                 int brightness = 255 - absoluteDepth * 10;

//                 // Add shadow from neighbor
//                 if (getHeightAt(t.gridX - 1, t.gridY) > t.height)
//                     brightness -= 40;

//                 brightness = std::clamp(brightness, 20, 255);

//                 SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);
//                 SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
//             }


//             SDL_SetTextureColorMod(wallTex, 255, 255, 255); // reset
//         }

//         for (auto [dx, dy] : { std::pair{1, 0}, std::pair{0, 1} }) { // right and bottom neighbors
//             int nx = t.gridX + dx;
//             int ny = t.gridY + dy;
//             int neighborH = getHeightAt(nx, ny);

//             int heightDiff = t.height - neighborH;
//             if (heightDiff > 0) {
//                 for (int h = 1; h <= heightDiff * tilesPerHeight; ++h) {
//                     SDL_Rect cliffDst = {
//                         isoX,
//                         topY + h * verticalOverlap,
//                         tileWidth,
//                         tileHeight
//                     };

//                     int fakeDepth = -neighborH * tilesPerHeight + (h - 1);
//                     int brightness = 255 - fakeDepth * 10;
//                     brightness = std::clamp(brightness, 40, 255);

//                     SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);

//                     SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
//                 }
//                 SDL_SetTextureColorMod(wallTex, 255, 255, 255);
//             }
//         }

//         bool shadowed = getHeightAt(t.gridX - 1, t.gridY) > t.height;
//         int brightness = 180 + t.height * 20;  // High = bright, Low = dark

//         if (getHeightAt(t.gridX - 1, t.gridY) > t.height)
//             brightness -= 40;

//         brightness = std::clamp(brightness, 40, 255);
//         SDL_SetTextureColorMod(topTex, brightness, brightness, brightness);

//         SDL_Rect topDst = { isoX, topY, tileWidth, tileHeight };
//         SDL_RenderCopy(renderer, topTex, nullptr, &topDst);
//     }
// }

void World::render(int scrollX, int scrollY) {
    const int tileWidth = 64;
    const int tileHeight = 32;
    const int verticalOverlap = tileHeight / 4;
    const int tilesPerHeight = 4;

    int scaledTileWidth = tileWidth * zoom;
    int scaledTileHeight = tileHeight * zoom;
    int scaledVerticalOverlap = verticalOverlap * zoom;

    std::sort(tiles.begin(), tiles.end(), [](const TileInstance& a, const TileInstance& b) {
        return (a.gridX + a.gridY) < (b.gridX + b.gridY);
    });

    for (const auto& t : tiles) {
        SDL_Texture* topTex = grassTexture;
        SDL_Texture* wallTex = cliffTexture;

        int baseX = (t.gridX - t.gridY) * (tileWidth / 2);
        int baseY = (t.gridX + t.gridY) * (tileHeight / 2);
        int isoX = (baseX - scrollX) * zoom;
        int isoY = (baseY - scrollY) * zoom;
        int topY = isoY - t.height * tilesPerHeight * scaledVerticalOverlap;

        // MOUNTAIN WALLS
        if (t.height > 0) {
            for (int h = t.height * tilesPerHeight; h >= 1; --h) {
                SDL_Rect cliffDst = {
                    isoX,
                    topY + h * scaledVerticalOverlap,
                    scaledTileWidth,
                    scaledTileHeight
                };
                SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
            }
        }
        // VALLEY WALLS
        else if (t.height < 0) {
            int totalSubTiles = -t.height * tilesPerHeight;
            int baseDepth = -t.height;

            for (int s = 0; s <= totalSubTiles; ++s) {
                SDL_Rect cliffDst = {
                    isoX,
                    topY + s * scaledVerticalOverlap,
                    scaledTileWidth,
                    scaledTileHeight
                };

                int absoluteDepth = baseDepth * tilesPerHeight + s;
                int brightness = 255 - absoluteDepth * 10;

                if (getHeightAt(t.gridX - 1, t.gridY) > t.height)
                    brightness -= 40;

                brightness = std::clamp(brightness, 20, 255);
                SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);
                SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
            }

            SDL_SetTextureColorMod(wallTex, 255, 255, 255); // reset
        }

        // GAP-FILLING WALLS TO RIGHT/BOTTOM NEIGHBORS
        for (auto [dx, dy] : { std::pair{1, 0}, std::pair{0, 1} }) {
            int nx = t.gridX + dx;
            int ny = t.gridY + dy;
            int neighborH = getHeightAt(nx, ny);

            int heightDiff = t.height - neighborH;
            if (heightDiff > 0) {
                for (int h = 1; h <= heightDiff * tilesPerHeight; ++h) {
                    SDL_Rect cliffDst = {
                        isoX,
                        topY + h * scaledVerticalOverlap,
                        scaledTileWidth,
                        scaledTileHeight
                    };

                    int fakeDepth = -neighborH * tilesPerHeight + (h - 1);
                    int brightness = 255 - fakeDepth * 10;
                    brightness = std::clamp(brightness, 40, 255);

                    SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);
                    SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
                }
                SDL_SetTextureColorMod(wallTex, 255, 255, 255);
            }
        }

        // 🌥️ TOP TILE SHADOW AND BRIGHTNESS
        int brightness = 180 + t.height * 20;
        if (getHeightAt(t.gridX - 1, t.gridY) > t.height)
            brightness -= 40;

        brightness = std::clamp(brightness, 40, 255);
        SDL_SetTextureColorMod(topTex, brightness, brightness, brightness);

        SDL_Rect topDst = { isoX, topY, scaledTileWidth, scaledTileHeight };
        SDL_RenderCopy(renderer, topTex, nullptr, &topDst);
    }
}



int World::getHeightAt(int x, int y) {
    for (const auto& t : tiles) {
        if (t.gridX == x && t.gridY == y)
            return t.height;
    }
    return 0;
}
