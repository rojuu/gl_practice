#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "GL/glew.h"

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
    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, 
        SDL_GL_CONTEXT_PROFILE_CORE
    );

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetSwapInterval(1);

    #ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
	#endif


    glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

    bool running = true;
    while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT){
				running = false;
            }

			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_r:
					glClearColor(1.0, 0.0, 0.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(window);
					break;
				case SDLK_g:
					glClearColor(0.0, 1.0, 0.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(window);
					break;
				case SDLK_b:
					glClearColor(0.0, 0.0, 1.0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);
					SDL_GL_SwapWindow(window);
					break;
				default:
					break;
				}
			}
		}
	}

	SDL_GL_DeleteContext(context);

	SDL_DestroyWindow(window);

	SDL_Quit();
    return 0;
}
