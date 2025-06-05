#include "renderer.hpp"
#include <SDL2/SDL_image.h>
#include <stdexcept>

Renderer::Renderer(const std::string& title, int width, int height)
    : window(nullptr), renderer(nullptr), width(width), height(height) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL2");

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        throw std::runtime_error("Failed to initialize SDL2_image");

    window = SDL_CreateWindow(title.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height, 0);
    if (!window)
        throw std::runtime_error("Failed to create SDL window");

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        throw std::runtime_error("Failed to create SDL renderer");
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

SDL_Renderer* Renderer::getRenderer() const {
    return renderer;
}
