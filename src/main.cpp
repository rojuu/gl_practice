#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "SDL.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <cstdint>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/** TODO: 
 * 
 * Should we actually use windows.h rather than SDL?
 * If we'd do that, we could use some useful windows API calls.
 * Maybe still use SDL, but only use those calls for useful stuff?
 * How would I go about portability in a situation like that?
 * 
 **/

#define internal static

//#define arrayCount(arr) (sizeof(arr)/sizeof(*arr))
#define arrayCount(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#include "types.cpp"
#include "math.cpp"

#include "objects.h"

internal const f32 PI = glm::pi<f32>();

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

internal u32
loadShaders(const char *vertexFilePath, const char *fragmentFilePath) {
    // Create the shaders
    u32 vertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // TODO: is CRT better for file loading than C++ std library?
    // Should we do file loading in a completely different way?

    // TODO: Is std::string slow for this? Wouldn't it cause a lot of allocations?

    // Read the Vertex Shader code from the file
    std::string vertexShaderCode;
    std::ifstream vertexShaderStream(vertexFilePath, std::ios::in);
    if(vertexShaderStream.is_open()) {
        std::string Line = "";
        while(getline(vertexShaderStream, Line)) {
            vertexShaderCode += "\n" + Line;
        }
        vertexShaderStream.close();
    } else {
        printf("Cannot open %s\n", vertexFilePath);
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string fragmentShaderCode;
    std::ifstream fragmentShaderStream(fragmentFilePath, std::ios::in);
    if(fragmentShaderStream.is_open()) {
        std::string Line = "";
        while(getline(fragmentShaderStream, Line)) {
            fragmentShaderCode += "\n" + Line;
        }
        fragmentShaderStream.close();
    } else {
        printf("Cannot open %s\n", fragmentFilePath);
        return 0;
    }

    i32 result = 0;
    i32 infoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertexFilePath);
    char const *vertexSourcePointer = vertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
    glCompileShader(vertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
        printf("%s\n", &vertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragmentFilePath);
    char const *fragmentSourcePointer = fragmentShaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer, NULL);
    glCompileShader(fragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0) {
        std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
        printf("%s\n", &fragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    u32 programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        printf("%s\n", &programErrorMessage[0]);
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

internal u32
loadTexture(const char *filename, bool flipVerticallyOnLoad, GLint internalFormat, GLenum format) {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    i32 width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flipVerticallyOnLoad);
    u8 *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load texture\n");
        return 0;
    }
    stbi_image_free(data);
    return texture;
}

internal inline u32
loadTextureJPG(const char *filename, bool flipVerticallyOnLoad) {
    u32 texture = loadTexture(filename, flipVerticallyOnLoad, GL_RGB, GL_RGB);
    return texture;
}

internal inline u32
loadTexturePNG(const char *filename, bool flipVerticallyOnLoad) {
    u32 texture = loadTexture(filename, flipVerticallyOnLoad, GL_RGBA, GL_RGBA);
    return texture;
}

//TODO: Should these set uniform functions do glUseProgram every time?
// I got confused, because I wasn't doing that before using these functions.
internal void
setUniformM4(u32 shader, const char *name, m4 matrix) {
    u32 handle = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(handle, 1, GL_FALSE, glm::value_ptr(matrix));
}

internal void
setUniform3f(u32 shader, const char *name, f32 f1, f32 f2, f32 f3) {
    u32 handle = glGetUniformLocation(shader, name);
    glUniform3f(handle, f1, f2, f3);
}

internal void
setUniformV3(u32 shader, const char *name, v3 v) {
    setUniform3f(shader, name, v.x, v.y, v.z);
}

i32
main(i32 argc, char **argv) {
    // Init SDL stuff
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

    SDL_Window *window = SDL_CreateWindow(
        "GL_TEST",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL);

    if(!window) {
        printf("SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Init OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_GL_SetSwapInterval(0);

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Load shaders
    u32 basicShader = loadShaders("data/shaders/basic.v", "data/shaders/basic.f");
    u32 lightShader = loadShaders("data/shaders/basic.v", "data/shaders/light.f");
    if(!basicShader ||
       !lightShader) {
        printf("Error loading shaders.\n");
        return -1;
    }

//Load textures
#if 0
    u32 texture0 = loadTextureJPG("data/textures/container.jpg", true);
    u32 texture1 = loadTexturePNG("data/textures/awesomeface.png", true);
    if(!texture0 ||
       !texture1) {
        printf("Error loading textures.\n");
        return -1;
    }
#endif

//Quads
#if 0
    v3 quadPositions[]{
        v3(0, 0, 0),
        v3(1, 0, 0),
        v3(0, 1, 0),
        v3(0, 0, 0),
    };
    Rotation quadRotations[]{
        {v3(0, 0, 1), glm::radians((float)0)},
        {v3(0, 0, 1), glm::radians((float)0)},
        {v3(0, 0, 1), glm::radians((float)30)},
        {v3(0, 0, 1), glm::radians((float)60)},
    };
    v3 quadScales[]{
        v3(1.0f, 1.0f, 1.0f),
        v3(0.1f, 0.1f, 1.0f),
        v3(0.5f, 0.5f, 1.0f),
        v3(2.5f, 0.4f, 1.0f),
    };

    const i32 quadCount = 1;
    // const i32 quadCount = arrayCount(quadPositions);
    Mesh quadMeshArray[quadCount];

    u32 quadVertexArray;
    glGenVertexArrays(1, &quadVertexArray);

    u32 quadElementBuffer;
    glGenBuffers(1, &quadElementBuffer);

    u32 quadVertexBuffer;
    glGenBuffers(1, &quadVertexBuffer);
    u32 quadColorBuffer;
    glGenBuffers(1, &quadColorBuffer);
    u32 quadTexCoordBuffer;
    glGenBuffers(1, &quadTexCoordBuffer);

    glBindVertexArray(quadVertexArray);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexPositions), quadVertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, quadColorBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexColors), quadVertexColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, quadTexCoordBuffer);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCoords), quadTexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);

    for(i32 i = 0; i < quadCount; i++) {
        quadMeshArray[i].vao           = quadVertexArray;
        quadMeshArray[i].count         = arrayCount(quadIndices);
        quadMeshArray[i].shaderProgram = basicShader;
    }
    //END Quads
#endif

//Cubes
#if 1
    v3 cubePositions[]{
        v3(0.0f, 0.0f, 0.0f),
        // v3(2.0f, 5.0f, -15.0f),
        // v3(-1.5f, -2.2f, -2.5f),
        // v3(-3.8f, -2.0f, -12.3f),
        // v3(2.4f, -0.4f, -3.5f),
        // v3(-1.7f, 3.0f, -7.5f),
        // v3(1.3f, -2.0f, -2.5f),
        // v3(1.5f, 2.0f, -2.5f),
        // v3(1.5f, 0.2f, -1.5f),
        // v3(-1.3f, 1.0f, -1.5f),
    };
    Rotation cubeRotations[]{
        {v3(0, 1, 0), glm::radians((float)30)},
        {v3(1, 1, 1), glm::radians((float)24)},
        {v3(0, 1, 1), glm::radians((float)30)},
        {v3(1, 1, 0), glm::radians((float)60)},
        {v3(1, 1, 0), glm::radians((float)60)},
        {v3(0, 1, 1), glm::radians((float)30)},
        {v3(1, 1, 1), glm::radians((float)24)},
        {v3(0, 1, 1), glm::radians((float)30)},
        {v3(0, 1, 1), glm::radians((float)30)},
        {v3(1, 0, 1), glm::radians((float)10)},
    };
    const i32 cubeCount = arrayCount(cubePositions);
    Mesh cubeMeshArray[cubeCount];

    u32 cubeVertexArray;
    glGenVertexArrays(1, &cubeVertexArray);

    u32 cubeVertexBuffer;
    glGenBuffers(1, &cubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);

    u32 cubeNormalBuffer;
    glGenBuffers(1, &cubeNormalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);

    // u32 cubeColorBuffer;
    // glGenBuffers(1, &cubeColorBuffer);
    // u32 cubeTexCoordBuffer;
    // glGenBuffers(1, &cubeTexCoordBuffer);

    //Make cube vertex array
    glBindVertexArray(cubeVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, cubeColorBuffer);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexColors), cubeVertexColors, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, cubeTexCoordBuffer);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexCoords), cubeTexCoords, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(2);

    //Make light vertex array
    u32 lightVertexArray;
    glGenVertexArrays(1, &lightVertexArray);

    glBindVertexArray(lightVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //Cube transform data;
    const int cubeTriangleCount = arrayCount(cubeVertexPositions) / 3;

    for(i32 i = 0; i < cubeCount; ++i) {
        cubeMeshArray[i].vao           = cubeVertexArray;
        cubeMeshArray[i].count         = cubeTriangleCount;
        cubeMeshArray[i].shaderProgram = basicShader;
    }

        // glUseProgram(basicShader);
        // glUniform1i(glGetUniformLocation(basicShader, "inTexture0"), 0);
        // glUniform1i(glGetUniformLocation(basicShader, "inTexture1"), 1);
#endif
    //END cubes

    v3 lightPos = v3(1.2f, 1.0f, 2.0f);
    v3 viewPos  = v3(0.0f, 2.0f, 3.0f);

    glUseProgram(basicShader);
    setUniform3f(basicShader, "objectColor", 1.0f, 0.5f, 0.31f);
    setUniform3f(basicShader, "lightColor", 1.0f, 1.0f, 1.0f);
    setUniformV3(basicShader, "lightPos", lightPos);
    setUniformV3(basicShader, "viewPos", viewPos);

    b32 running     = true;
    f64 currentTime = (f32)SDL_GetPerformanceCounter() /
                      (f32)SDL_GetPerformanceFrequency();
    f64 lastTime       = 0;
    f64 deltaTime      = 0;
    i32 frameCounter   = 0;
    i32 lastFrameCount = 0;
    f64 lastFpsTime    = 0;
    while(running) {
        lastTime    = currentTime;
        currentTime = (f64)SDL_GetPerformanceCounter() /
                      (f64)SDL_GetPerformanceFrequency();
        deltaTime = (f64)(currentTime - lastTime);

        // Count frames for every second and print it as the title of the window
        ++frameCounter;
        if(currentTime >= (lastFpsTime + 1.f)) {
            lastFpsTime     = currentTime;
            i32 deltaFrames = frameCounter - lastFrameCount;
            lastFrameCount  = frameCounter;
            char title[64];
            sprintf(title, "FPS: %d", deltaFrames);
            SDL_SetWindowTitle(window, title);
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT: {
                running = false;
            } break;

            case SDL_KEYDOWN: {
                switch(event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    running = false;
                } break;

                case '1': {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                } break;

                case '2': {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } break;
                }
            } break;
            }
        }

        // glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m4 projection = glm::perspective(glm::radians(45.0f),
                                         (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);

        // m4 view = glm::lookAt(
        // 	fp.position,
        // 	fp.position + fp.direction,
        // 	v3(0,1,0)
        // );

        m4 view = glm::lookAt(
            viewPos,
            v3(0, 0, 0),
            v3(0, 1, 0));

// Draw quads
#if 0
        for (i32 i = 0; i < quadCount; i++)
        {
            v3 scale = quadScales[i];
            v3 position = quadPositions[i];
            Mesh mesh = quadMeshArray[i];
            Rotation rotation = quadRotations[i];

            m4 m = m4(1.0f);
            m4 translate = glm::translate(m, position);
            m4 rotate = glm::rotate(m, rotation.angle, rotation.axis);
            rotate = glm::rotate(rotate, glm::radians(10.f), v3(1, 0, 0));
            m4 scaleM = glm::scale(m, scale);
            m4 model = scaleM * translate * rotate;
            m4 mvp = projection * view * model;

            u32 mvpHandle = glGetUniformLocation(mesh.shaderProgram, "MVP");
            glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, glm::value_ptr(mvp));

            glUseProgram(mesh.shaderProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, 0);
        }
#endif

// Rotate cubes
#if 1
        for(i32 i = 0; i < cubeCount; i += 1) {
            float rotationAmount   = deltaTime;
            cubeRotations[i].angle = fmod(cubeRotations[i].angle + rotationAmount, (PI * 2));
        }

            // for(i32 i = 1; i < cubeCount; i += 3) {
            //     float rotationAmount   = -deltaTime;
            //     cubeRotations[i].angle = fmod(cubeRotations[i].angle + rotationAmount, (PI * 2));
            // }

            // for(i32 i = 2; i < cubeCount; i += 3) {
            //     float rotationAmount   = 0.5f * deltaTime;
            //     cubeRotations[i].angle = fmod(cubeRotations[i].angle + rotationAmount, (PI * 2));
            // }
#endif

// Draw light source
#if 1
        {
            m4 model = m4(1.0f);
            model    = glm::translate(model, lightPos);
            model    = glm::scale(model, v3(0.3f));

            glUseProgram(lightShader);

            setUniformM4(lightShader, "model", model);
            setUniformM4(lightShader, "view", view);
            setUniformM4(lightShader, "projection", projection);

            glBindVertexArray(lightVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, cubeTriangleCount);
        }
#endif

// Draw cubes
#if 1
        for(i32 i = 0; i < cubeCount; i++) {
            // v3 scale = cubeScales[i];
            v3 scale          = v3(1.0f);
            v3 position       = cubePositions[i];
            Mesh mesh         = cubeMeshArray[i];
            Rotation rotation = cubeRotations[i];

            m4 model = m4(1.0f);
            model    = glm::translate(model, position);
            model    = glm::rotate(model, rotation.angle, rotation.axis);
            model    = glm::scale(model, scale);
            // m4 mvp       = projection * view * model;

            glUseProgram(mesh.shaderProgram);

            setUniformM4(mesh.shaderProgram, "model", model);
            setUniformM4(mesh.shaderProgram, "view", view);
            setUniformM4(mesh.shaderProgram, "projection", projection);

            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, texture0);
            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, texture1);
            glBindVertexArray(mesh.vao);
            glDrawArrays(GL_TRIANGLES, 0, mesh.count);
        }
#endif

        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    return 0;
}
