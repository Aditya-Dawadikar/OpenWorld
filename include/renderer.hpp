#pragma once
#include <SDL2/SDL.h>
#include <string>

class Renderer {
public:
    Renderer(const std::string& title, int width, int height);
    ~Renderer();

    void clear();
    void present();

    SDL_Renderer* getRenderer() const;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width, height;
};
