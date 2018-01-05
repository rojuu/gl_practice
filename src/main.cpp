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

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef i8  b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef glm::vec2 v2;
typedef glm::vec3 v3;
typedef glm::vec4 v4;

typedef glm::mat2 m2;
typedef glm::mat3 m3;
typedef glm::mat4 m4;

#define internal static

//#define arrayCount(arr) (sizeof(arr)/sizeof(*arr))
#define arrayCount(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

internal const f32 PI = glm::pi<f32>();

internal u32
loadShaders(const char * vertexFilePath, const char * fragmentFilePath) {

	// Create the shaders
	u32 vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	u32 fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

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

	GLint result = GL_FALSE;
	i32 infoLogLength;

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
	u32 programID = glCreateProgram();
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

struct FPCamera {
	v3 position;
	v3 direction;
	f32 yaw, pitch;
	f32 speed;
	f32 eyeHeight;
};

struct KeyboardInput {
	b32 left, right, up, down;
};

struct Mesh {
	u32 vao;
	u32 count;
};

internal inline f32
clamp(f32 value, f32 min, f32 max) {
	if(value < min) value = min;
	if(value > max) value = max;
	return value;
}

// Normalize or zero
internal inline v3
noz(v3 input) {
	v3 result = input.x||input.y||input.z ? glm::normalize(input) : v3(0,0,0);
	return result;
}

internal inline v2
noz(v2 input) {
	v2 result = input.x||input.y ? glm::normalize(input) : v2(0,0);
	return result;
}

internal v3
rotate(v3 in, v3 axis, f32 theta) {
	f32 cosTheta = glm::cos(theta);
	f32 sinTheta = glm::sin(theta);
	return (in * cosTheta) + (glm::cross(axis, in) * sinTheta) + (axis * glm::dot(axis, in)) * (1 - cosTheta);
}

inline f32
angleBetween(v3 a, v3 b) {
	v3 da=noz(a);
	v3 db=noz(b);
	return glm::acos(glm::dot(da, db));
}

inline f32
angleBetween(v3 a, v3 b, v3 origin) {
	v3 da=noz(a-origin);
	v3 db=noz(b-origin);
	return glm::acos(glm::dot(da, db));
}

internal inline v3
sphericalToCartesian(f32 radius, f32 longtitude, f32 latitude) {
	f32 x = radius * glm::cos(latitude) * glm::sin(longtitude);
	f32 y = radius * glm::sin(latitude);
	f32 z = radius * glm::cos(latitude) * glm::cos(longtitude);
	return v3(x,y,z);
}

i32
main(i32 argc, char **argv) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Error: %s\n", SDL_GetError());
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

	if(!window) {
		printf("SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

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

	u32 shaderProgram = loadShaders("shaders/basic.v", "shaders/basic.f");
	if(shaderProgram == 0){
		printf("Error loading shaders.\n");
		return -1;
	}

	const f32 triangleVertexBuffer[] = {
		 -0.5f, -0.5f,  0.0f,
		  0.5f, -0.5f,  0.0f,
		  0.0f,  0.5f,  0.0f,
	};

	const f32 triangleColorBuffer[] = {
		 0.0f, 0.0f, 1.0f,
		 0.0f, 1.0f, 0.0f,
		 1.0f, 0.0f, 0.0f,
	};

	// TODO: Update the object count when you add more objects to draw
	const i32 objectCount = 1;
	Mesh meshArray[objectCount];

	// Init triangle
	{
		u32 vertexArray;
		glGenVertexArrays(1, &vertexArray);

		u32 vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		u32 colorBuffer;
		glGenBuffers(1, &colorBuffer);

		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertexBuffer), triangleVertexBuffer, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorBuffer), triangleColorBuffer, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);

		meshArray[0].vao = vertexArray;
		meshArray[0].count = arrayCount(triangleVertexBuffer) / 3;
	}	

	v3 cameraPos = v3(0,0,0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

	FPCamera fp;
	fp.position = v3(0, 0, -3);
	fp.pitch = 0.f;
	fp.yaw = 90.f;
	fp.speed = 10.f;
	fp.eyeHeight = 1.8f;

	KeyboardInput keyboardInput = {};

	f64 currentTime = (f32)SDL_GetPerformanceCounter() /
							(f32)SDL_GetPerformanceFrequency();
	f64 lastTime = 0;
	f64 deltaTime = 0;
	i32 frameCounter = 0;
	i32 lastFrameCount = 0;
	b32 running = true;
	f64 lastFpsTime = 0;
	while (running) {
		lastTime = currentTime;
		currentTime = (f64)SDL_GetPerformanceCounter() /
						(f64)SDL_GetPerformanceFrequency();
		deltaTime = (f64)(currentTime - lastTime);

		// Count frames for every second and print it as the title of the window
		++frameCounter;
		if(currentTime >= (lastFpsTime + 1.f)) {
			lastFpsTime = currentTime;
			i32 deltaFrames = frameCounter - lastFrameCount;
			lastFrameCount = frameCounter;
			char title[64];
			sprintf(title, "FPS: %d", deltaFrames);
			SDL_SetWindowTitle(window, title);
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					running = false;
				} break;

				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE: {
							running = false;
						} break;

						case '1': {
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						} break;

						case '2': {
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						} break;

						case 'w': {
							keyboardInput.up = true;
						} break;

						case 's': {
							keyboardInput.down = true;
						} break;

						case 'a': {
							keyboardInput.left = true;
						} break;

						case 'd': {
							keyboardInput.right = true;
						} break;
					}
				} break;

				case SDL_KEYUP: {
					switch (event.key.keysym.sym) {
						case 'w': {
							keyboardInput.up = false;
						} break;

						case 's': {
							keyboardInput.down = false;
						} break;

						case 'a': {
							keyboardInput.left = false;
						} break;

						case 'd': {
							keyboardInput.right = false;
						} break;
					}
				}

				case SDL_MOUSEMOTION: {
					// TODO: Investigate, why pressing w/a/s/d causes really high motion values, in mousemotion
					if(event.motion.yrel > 100) {
						break;
					}
					if(event.motion.xrel > 100) {
						break;
					}
					f32 sens = 0.1f;
					f32 lim = 0.01f;
					fp.yaw += (f32)event.motion.xrel * sens;
					fp.yaw = fmod(fp.yaw, 360.f);
					fp.pitch -= (f32)event.motion.yrel * sens;
					fp.pitch = clamp(fp.pitch, -90.f + lim, 90.f - lim);
				} break;
			}
		}

		// Movement
		#if 0
		{
			fp.direction.x = cos(glm::radians(fp.pitch)) * cos(glm::radians(fp.yaw));
			fp.direction.y = sin(glm::radians(fp.pitch));
			fp.direction.z = cos(glm::radians(fp.pitch)) * sin(glm::radians(fp.yaw));
			fp.direction = noz(fp.direction);

			float y = 0.f;
			float x = 0.f;
			y+=(f32)keyboardInput.up;
			y-=(f32)keyboardInput.down;
			x+=(f32)keyboardInput.right;
			x-=(f32)keyboardInput.left;

			v2 input = noz(v2(x, y));

			fp.position += fp.direction * input.y * fp.speed * deltaTime;
			fp.position += glm::cross(fp.direction, v3(0,1,0)) * input.x * fp.speed * deltaTime;
			fp.position.y = fp.eyeHeight;

			v3* pos = &fp.position;
			// printf("pitch %f, yaw %f, pos.x %f, pos.y %f, pos.z %f\r", fp.pitch, fp.yaw, pos->x, pos->y, pos->z);
		} 
		#endif

		// glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m4 projection = glm::perspective(glm::radians(90.0f),
			(f32) SCREEN_WIDTH / (f32) SCREEN_HEIGHT, 0.1f, 100.0f);

		// m4 view = glm::lookAt(
		// 	fp.position,
		// 	fp.position + fp.direction,
		// 	v3(0,1,0)
		// );

		m4 view = glm::lookAt(
			v3(0,0,1),
			v3(0,0,0),
			v3(0,1,0)
		);

		m4 model = glm::translate(m4(1.0f), v3(0,0,0));
		m4 mvp = projection * view * model;

		u32 mvpHandle = glGetUniformLocation(shaderProgram, "MVP");
		glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, glm::value_ptr(mvp));

		// Draw stuff
		for(i32 i = 0; i < objectCount; i++) {
			glUseProgram(shaderProgram);
			glBindVertexArray(meshArray[i].vao);
			glDrawArrays(GL_TRIANGLES, 0, meshArray[i].count);
		}

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	return 0;
}
