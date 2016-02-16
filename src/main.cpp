#include <iostream>

#include <SDL/SDL.h>

#include "triangle.cpp"

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);

    auto pWindow = SDL_CreateWindow("c2baVulkanTriangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);

    VulkanExample triangle;
    triangle.initSwapchain();
    triangle.prepare();
    //triangle.renderLoop();

    auto done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                done = true;
            }
        }

        triangle.render();
    }

    SDL_Quit();

    return 0;
}