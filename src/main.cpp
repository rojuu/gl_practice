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
#include "glm/gtx/transform.hpp"

#include "util.h"

#define internal static

internal const float PI = glm::pi<float>();

internal GLuint
loadShaders(const char * vertexFilePath, const char * fragmentFilePath) {

	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vertexFilePath, std::ios::in);
	if(vertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(vertexShaderStream, Line))
			vertexShaderCode += "\n" + Line;
		vertexShaderStream.close();
	}else{
		println("Cannot open %s", vertexFilePath);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderStream(fragmentFilePath, std::ios::in);
	if(fragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(fragmentShaderStream, Line))
			fragmentShaderCode += "\n" + Line;
		fragmentShaderStream.close();
	}else{
		println("Cannot open %s", fragmentFilePath);
		return 0;
	}

	GLint result = GL_FALSE;
	int infoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexFilePath);
	char const * vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer , NULL);
	glCompileShader(vertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0){
		std::vector<char> vertexShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
		printf("%s\n", &vertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragmentFilePath);
	char const * fragmentSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer , NULL);
	glCompileShader(fragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( infoLogLength > 0 ){
		std::vector<char> fragmentShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
		printf("%s\n", &fragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0){
		std::vector<char> programErrorMessage(infoLogLength+1);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
		printf("%s\n", &programErrorMessage[0]);
	}


	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// An array of 3 vectors which represents 3 vertices
internal const GLfloat vertexBufferTriangle[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

internal const GLfloat vertexBufferCube[] = {
	-1.0f,-1.0f,-1.0f, // triangle 1 : begin
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, // triangle 1 : end
	1.0f, 1.0f,-1.0f, // triangle 2 : begin
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f, // triangle 2 : end
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
};

// One color for each vertex. They were generated randomly.
internal GLfloat colorBufferCube[] = {
	0.583f,  0.771f,  0.014f,
	0.609f,  0.115f,  0.436f,
	0.327f,  0.483f,  0.844f,
	0.822f,  0.569f,  0.201f,
	0.435f,  0.602f,  0.223f,
	0.310f,  0.747f,  0.185f,
	0.597f,  0.770f,  0.761f,
	0.559f,  0.436f,  0.730f,
	0.359f,  0.583f,  0.152f,
	0.483f,  0.596f,  0.789f,
	0.559f,  0.861f,  0.639f,
	0.195f,  0.548f,  0.859f,
	0.014f,  0.184f,  0.576f,
	0.771f,  0.328f,  0.970f,
	0.406f,  0.615f,  0.116f,
	0.676f,  0.977f,  0.133f,
	0.971f,  0.572f,  0.833f,
	0.140f,  0.616f,  0.489f,
	0.997f,  0.513f,  0.064f,
	0.945f,  0.719f,  0.592f,
	0.543f,  0.021f,  0.978f,
	0.279f,  0.317f,  0.505f,
	0.167f,  0.620f,  0.077f,
	0.347f,  0.857f,  0.137f,
	0.055f,  0.953f,  0.042f,
	0.714f,  0.505f,  0.345f,
	0.783f,  0.290f,  0.734f,
	0.722f,  0.645f,  0.174f,
	0.302f,  0.455f,  0.848f,
	0.225f,  0.587f,  0.040f,
	0.517f,  0.713f,  0.338f,
	0.053f,  0.959f,  0.120f,
	0.393f,  0.621f,  0.362f,
	0.673f,  0.211f,  0.457f,
	0.820f,  0.883f,  0.371f,
	0.982f,  0.099f,  0.879f
};

internal inline float
clamp(float value, float min, float max) {
	if(value < min) value = min;
	if(value > max) value = max;
	return value;
}

internal inline glm::vec3
sphericalToCartesian(float radius, float longtitude, float latitude) {
	float x = radius * glm::cos(latitude) * glm::cos(longtitude);
	float y = radius * glm::sin(latitude);
	float z = radius * glm::cos(latitude) * glm::sin(longtitude) * -1;
	return glm::vec3(x,y,z);
}

int
main(int argc, char **argv) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		println("SDL_Error: %c\n", SDL_GetError());
		return -1;
	}

	atexit(SDL_Quit);

	SDL_Window* window = SDL_CreateWindow(
		"GL_TEST",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_OPENGL
	);

	if(!window){
		println("SDL_Error: %c\n", SDL_GetError());
		return -1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_GLContext context = SDL_GL_CreateContext(window);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);	

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

	GLuint programID = loadShaders("shaders/basic.v", "shaders/basic.f");
	if(programID == 0){
		println("Error loading shaders.");
		return -1;
	}

	srand(time(NULL));

	for (int v = 0; v < 12*3; v++) {
		colorBufferCube[3*v+0] = (float)rand() / (float)RAND_MAX;
		colorBufferCube[3*v+1] = (float)rand() / (float)RAND_MAX;
		colorBufferCube[3*v+2] = (float)rand() / (float)RAND_MAX;
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferCube), vertexBufferCube, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorBufferCube), colorBufferCube, GL_STATIC_DRAW);


	glm::vec3 cameraVector = glm::vec3(0,0,0);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

	float radius = 10, longtitude = 0, latitude = 0.08;

	float currentTime = (float)SDL_GetPerformanceCounter() /
							(float)SDL_GetPerformanceFrequency();
	float lastTime = 0;
	float deltaTime = 0;
	bool running = true;
	while (running) {
		lastTime = currentTime;
		currentTime = (float)SDL_GetPerformanceCounter() /
						(float)SDL_GetPerformanceFrequency();
		deltaTime = (float)(currentTime - lastTime);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					running = false;
				} break;

				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
							break;
					}
				} break;

				case SDL_MOUSEMOTION: {
					float sens = 0.005f;
					float lim = 0.01f;
					longtitude -= (float)event.motion.xrel * sens;
					latitude += (float)event.motion.yrel * sens;
					latitude = clamp(latitude, -PI/2 + lim, PI/2 - lim);
				} break;

				case SDL_MOUSEWHEEL: {
					radius -= (float)event.wheel.y;
					radius = clamp(radius, 1, 100);
				} break;
			}
		}

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 projection = glm::perspective(glm::radians(90.0f),
			(float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f, 100.0f);


		cameraVector = sphericalToCartesian(radius, longtitude, latitude);
		glm::mat4 view = glm::lookAt(
			cameraVector,
			glm::vec3(0,0,0),
			glm::vec3(0,1,0)
		);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
		glm::mat4 mvp = projection * view * model;

		GLuint mvpHandle = glGetUniformLocation(programID, "MVP");

		glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, &mvp[0][0]);

		glUseProgram(programID);

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
		glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
		glDisableVertexAttribArray(0);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                  // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		#if 0
		printf("deltatime: %f\n", deltaTime);
		for (int v = 0; v < 12*3; v++) {
			colorBufferCube[3*v+0] = fmod(colorBufferCube[3*v+0] + deltaTime, 1.f);
			colorBufferCube[3*v+1] = fmod(colorBufferCube[3*v+1] + deltaTime, 1.f);
			colorBufferCube[3*v+2] = fmod(colorBufferCube[3*v+2] + deltaTime, 1.f);
		}
		#endif

		glBufferData(GL_ARRAY_BUFFER, sizeof(colorBufferCube), colorBufferCube, GL_STATIC_DRAW);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	return 0;
}
