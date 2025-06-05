#include "world.hpp"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <queue>
#include "globals.hpp"
#include <set>

World::World(SDL_Renderer* renderer, int width, int height)
    : renderer(renderer), width(width), height(height) {

    grassTexture = loadTexture("../assets/grass.png");
    waterTexture = loadTexture("../assets/water.png");
    rockTexture  = loadTexture("../assets/dirt.png");
    cliffTexture = loadTexture("../assets/rock.png");

    
    heightMap = std::vector<std::vector<int>>(width, std::vector<int>(height));
    typeMap = std::vector<std::vector<TileType>>(width, std::vector<TileType>(height, TILE_GRASS));

    featureMask = std::vector<std::vector<bool>>(width, std::vector<bool>(height, false));
    valleySeed = std::vector<std::vector<bool>>(width, std::vector<bool>(height, false));
    lakeSeed = std::vector<std::vector<bool>>(width, std::vector<bool>(height, false));

    generateWorld();
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
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // force pixelated
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);

    return texture;
}

void World::generateWorld() {
    srand(SDL_GetTicks());
    tiles.clear();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (rand()%100 > 75){
                heightMap[x][y] = (rand() % 3) - 1;  // yields -1, 0, or 1
            }else{
                heightMap[x][y] = 0;
            }
        }
    }

    generateMountains(5, 6, 4, 10, 6);
    generateValleys(5, 3, 6);
    
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

void World::generateMountains(int numPlateaus, int plateauRadius, int minHeight, int maxHeight, int falloffRadius){

    int range = maxHeight - minHeight + 1;

    for (int i = 0; i < numPlateaus; ++i) {
        int centerX = rand() % width;
        int centerY = rand() % height;

        if (featureMask[centerX][centerY] == true){
            i--;
            continue;
        }

        int peakH = rand()%range + minHeight;

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

void World::generateValleys(int numValleys, int minDepth, int maxDepth) {

    for (int i = 0; i < numValleys; ++i) {
        int centerX = rand() % width;
        int centerY = rand() % height;

        bool makeLake = rand() % 100 > 50;

        if (featureMask[centerX][centerY]) {
            i--; // retry
            continue;
        }

        
        int range = maxDepth - minDepth + 1;

        int peakH = -(rand()%range + minDepth);

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
                valleySeed[x][y] = true;
                if(makeLake){
                    lakeSeed[x][y] = true;
                }

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
                valleySeed[x][y] = true;

                if (makeLake){
                    lakeSeed[x][y] = true;
                }
            }

            if (h<0 && makeLake){
                // valleySeed[x][y] = true;
                lakeSeed[x][y] = true;
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

        int isoX = int((baseX - scrollX) * zoom + 0.5f);
        int isoY = int((baseY - scrollY) * zoom + 0.5f);
        int topY = isoY - int(t.height * tilesPerHeight * scaledVerticalOverlap + 0.5f);

        auto applyWallShadow = [&](int pixelY, int tileH, int x, int y) {
            int brightness = 255;

            // Depth-based darkness for valleys
            float tileHeightAtPixel = tileH + float(pixelY) / tilesPerHeight;
            if (tileHeightAtPixel < 0.0f)
                brightness -= pixelY * 10;

            // Base lighting bias (optional)
            brightness += 10;

            // Apply shadow from higher tiles in shadow-casting directions
            if (getHeightAt(x + 1, y) > tileH)     brightness -= 30; // Right tile casts shadow
            if (getHeightAt(x,     y - 1) > tileH) brightness -= 30; // Top tile casts shadow

            return std::clamp(brightness, 20, 255);
        };


        // MOUNTAIN WALLS
        if (t.height > 0) {
            for (int h = t.height * tilesPerHeight; h >= 1; --h) {
                SDL_Rect cliffDst = { isoX, topY + h * scaledVerticalOverlap, scaledTileWidth, scaledTileHeight };
                int brightness = applyWallShadow(h, t.height, t.gridX, t.gridY);
                SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);
                SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
            }
        }
        // VALLEY WALLS
        else if (t.height < 0) {
            int totalSubTiles = -t.height * tilesPerHeight;
            for (int s = 0; s <= totalSubTiles; ++s) {
                SDL_Rect cliffDst = { isoX, topY + s * scaledVerticalOverlap, scaledTileWidth, scaledTileHeight };
                int brightness = applyWallShadow(s, t.height, t.gridX, t.gridY);
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
                    SDL_Rect cliffDst = { isoX, topY + h * scaledVerticalOverlap, scaledTileWidth, scaledTileHeight };
                    int brightness = applyWallShadow(h, neighborH, t.gridX, t.gridY);
                    SDL_SetTextureColorMod(wallTex, brightness, brightness, brightness);
                    SDL_RenderCopy(renderer, wallTex, nullptr, &cliffDst);
                }
                SDL_SetTextureColorMod(wallTex, 255, 255, 255);
            }
        }

        // Top surface brightness
        int topBrightness = 180 + t.height * 20;
        if (getHeightAt(t.gridX - 1, t.gridY) > t.height)
            topBrightness -= 40;
        topBrightness = std::clamp(topBrightness, 40, 255);
        SDL_SetTextureColorMod(topTex, topBrightness, topBrightness, topBrightness);

        SDL_Rect topDst = { isoX - 2, topY - 2, scaledTileWidth + 3, scaledTileHeight + 3 };
        SDL_RenderCopy(renderer, topTex, nullptr, &topDst);

        // Render water surface
        if (lakeSeed[t.gridX][t.gridY]) {
            SDL_SetTextureBlendMode(waterTexture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(waterTexture, 204); // 80%
            SDL_SetTextureColorMod(waterTexture, 255, 255, 255);

            int waterY = int(((t.gridX + t.gridY) * (tileHeight / 2) - scrollY) * zoom + 0.5f);
            SDL_Rect waterDst = { isoX - 2, waterY - 2, scaledTileWidth + 2, scaledTileHeight + 2 };
            SDL_RenderCopy(renderer, waterTexture, nullptr, &waterDst);
            SDL_SetTextureAlphaMod(waterTexture, 255); // reset
        }
    }
}



int World::getHeightAt(int x, int y) {
    for (const auto& t : tiles) {
        if (t.gridX == x && t.gridY == y)
            return t.height;
    }
    return 0;
}
