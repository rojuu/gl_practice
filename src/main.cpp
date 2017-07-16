#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "GL/glew.h"
#include "glm/vec3.hpp"

#include "util.h"

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

int main(int argc, char **argv) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        println("SDL_Error: %c\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "GL_TEST", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
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

#if 0
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
#endif

    #ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
    #endif

    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

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
                default:
                    break;
                }
            }
        }

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        glDisableVertexAttribArray(0);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
