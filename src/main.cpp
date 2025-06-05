#include "renderer.hpp"
#include "world.hpp"
#include "player.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>

int main() {
    try {
        // Create renderer + SDL
        Renderer renderer("2.5D Pixel World", 640, 480);
        SDL_Renderer* sdlRenderer = renderer.getRenderer();
        World world(sdlRenderer);

        // Load player (uses 64x64 sprite frames)
        Player player(sdlRenderer, "../assets/archer_blond_hair.png", 64, 64);

        // Player's grid position in world
        int playerGridX = 5;
        int playerGridY = 5;

        // Tile size and screen center
        const int tileSize = 32;
        const int screenWidth = 640;
        const int screenHeight = 480;
        const int playerScreenX = screenWidth / 2 - 32; // 64px sprite
        const int playerScreenY = screenHeight / 2 - 32;

        // Camera scroll values
        int scrollX = playerGridX * tileSize;
        int scrollY = playerGridY * tileSize - playerGridX * 8;

        SDL_Event event;
        bool running = true;
        const int moveSpeed = 1;

        while (running) {
            int dx = 0, dy = 0;
            player.setMoving(false);  // Assume idle

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    running = false;
                else if (event.type == SDL_MOUSEWHEEL) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);

                    float oldZoom = world.zoom;

                    if (event.wheel.y > 0) {
                        world.zoom = std::min(world.zoom + 0.1f, 3.0f);
                    } else if (event.wheel.y < 0) {
                        world.zoom = std::max(world.zoom - 0.1f, 0.5f);
                    }

                    // Adjust scroll to zoom around cursor
                    float zoomRatio = world.zoom / oldZoom;
                    scrollX = (scrollX + mouseX) * zoomRatio - mouseX;
                    scrollY = (scrollY + mouseY) * zoomRatio - mouseY;
                }

                else if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:  dx = -1; break;
                        case SDLK_RIGHT: dx = 1;  break;
                        case SDLK_UP:    dy = -1; break;
                        case SDLK_DOWN:  dy = 1;  break;
                    }

                    if (dx != 0 || dy != 0) {
                        player.setMoving(true);  // Moving!
                        player.setDirection(dx, dy);

                        playerGridX += dx;
                        playerGridY += dy;

                        scrollX = playerGridX * 32;
                        scrollY = playerGridY * 32 - playerGridX * 8;
                    }
                }
            }

            player.update();

            renderer.clear();
            world.render(scrollX, scrollY);
            player.render(sdlRenderer, playerScreenX, playerScreenY);
            renderer.present();
        }


    } catch (const std::exception& e) {
        SDL_Log("Error: %s", e.what());
        return 1;
    }

    return 0;
}
