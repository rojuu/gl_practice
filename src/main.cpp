#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "util.h"

int main(int argc, char **argv) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        println("SDL_Error: %c\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "GL_TEST", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        512,
        512,
        SDL_WINDOW_OPENGL
    );

    if(!window){
        println("SDL_Error: %c\n", SDL_GetError());
        return -1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type){
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }
    }

    SDL_Quit();
    return 0;
}
