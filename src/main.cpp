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

const float pi = glm::pi<float>();

GLuint
LoadShaders(const char * vertex_file_path, const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		println("Cannot open %s", vertex_file_path);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}else{
		println("Cannot open %s", fragment_file_path);
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_triangle[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

static const GLfloat g_vertex_buffer_cube[] = {
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
static GLfloat g_color_buffer_cube[] = {
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

inline float 
clamp(float value, float min, float max) {
	if(value < min) value = min;
	if(value > max) value = max;
	return value;
}

inline glm::vec3
sphericalToCartesian(float radius, float longtitude, float latitude) {
	float x = radius * glm::cos(latitude) * glm::cos(longtitude);
	float y = radius * glm::sin(latitude);
	float z = radius * glm::cos(latitude) * glm::sin(longtitude) * -1;
	return glm::vec3(x,y,z);
}

void
ExitCleanUp() {
	SDL_Quit();
}

int
main(int argc, char **argv) {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		println("SDL_Error: %c\n", SDL_GetError());
		return -1;
	}

	atexit(ExitCleanUp);

	SDL_Window* window = SDL_CreateWindow(
		"GL_TEST",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_OPENGL
	);

	SDL_SetRelativeMouseMode(SDL_TRUE);

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

	GLuint programID = LoadShaders("shaders/basic.v", "shaders/basic.f");
	if(programID == 0){
		println("Error loading shaders.");
		return -1;
	}

	srand (time(NULL));

	for (int v = 0; v < 12*3; v++) {
		g_color_buffer_cube[3*v+0] = (float)rand() / (float)RAND_MAX;
		g_color_buffer_cube[3*v+1] = (float)rand() / (float)RAND_MAX;
		g_color_buffer_cube[3*v+2] = (float)rand() / (float)RAND_MAX;
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_cube), g_vertex_buffer_cube, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_cube), g_color_buffer_cube, GL_STATIC_DRAW);


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
					latitude = clamp(latitude, -pi/2 + lim, pi/2 - lim);
				} break;

				case SDL_MOUSEWHEEL: {
					radius -= (float)event.wheel.y;
					radius = clamp(radius, 1, 100);
				} break;
			}
		}

		printf("longt %f, lat %f, r %f\n", longtitude, latitude, radius);
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 Projection = glm::perspective(glm::radians(90.0f),
			(float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f, 100.0f);


		cameraVector = sphericalToCartesian(radius, longtitude, latitude);
		glm::mat4 View = glm::lookAt(
			cameraVector,
			glm::vec3(0,0,0),
			glm::vec3(0,1,0)
		);

		glm::mat4 Model = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
		glm::mat4 mvp = Projection * View * Model;

		GLuint mvp_handle = glGetUniformLocation(programID, "MVP");

		glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);

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

		SDL_GL_SwapWindow(window);

		#if 0
		for (int v = 0; v < 12*3; v++) {
			g_color_buffer_cube[3*v+0] = fmod(g_color_buffer_cube[3*v+0] + deltaTime, 1);
			g_color_buffer_cube[3*v+1] = fmod(g_color_buffer_cube[3*v+1] + deltaTime, 1);
			g_color_buffer_cube[3*v+2] = fmod(g_color_buffer_cube[3*v+2] + deltaTime, 1);
		}
		#endif

		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_cube), g_color_buffer_cube, GL_STATIC_DRAW);
	}

	SDL_GL_DeleteContext(context);

	SDL_DestroyWindow(window);
	return 0;
}
