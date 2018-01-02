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

// An array of 3 vectors which represents 3 vertices
internal const f32 vertexBufferTriangle[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

internal const f32 vertexBufferCube[] = {
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
internal f32 colorBufferCube[] = {
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

	SDL_GLContext context = SDL_GL_CreateContext(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetSwapInterval(1);

#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	u32 programID = loadShaders("shaders/basic.v", "shaders/basic.f");
	if(programID == 0){
		printf("Error loading shaders.\n");
		return -1;
	}

	srand(time(NULL));

	for (i32  v = 0; v < 12*3; v++) {
		colorBufferCube[3*v+0] = (f32)rand() / (f32)RAND_MAX;
		colorBufferCube[3*v+1] = (f32)rand() / (f32)RAND_MAX;
		colorBufferCube[3*v+2] = (f32)rand() / (f32)RAND_MAX;
	}

	u32 vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferCube), vertexBufferCube, GL_STATIC_DRAW);

	u32 colorBuffer;
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorBufferCube), colorBufferCube, GL_STATIC_DRAW);


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

	KeyboardInput ki = {};

	f32 currentTime = (f32)SDL_GetPerformanceCounter() /
							(f32)SDL_GetPerformanceFrequency();
	f32 lastTime = 0;
	f32 deltaTime = 0;
	b32 running = true;
	while (running) {
		lastTime = currentTime;
		currentTime = (f32)SDL_GetPerformanceCounter() /
						(f32)SDL_GetPerformanceFrequency();
		deltaTime = (f32)(currentTime - lastTime);

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
							ki.up = true;
						} break;

						case 's': {
							ki.down = true;
						} break;

						case 'a': {
							ki.left = true;
						} break;

						case 'd': {
							ki.right = true;
						} break;
					}
				} break;

				case SDL_KEYUP: {
					switch (event.key.keysym.sym) {
						case 'w': {
							ki.up = false;
						} break;

						case 's': {
							ki.down = false;
						} break;

						case 'a': {
							ki.left = false;
						} break;

						case 'd': {
							ki.right = false;
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
		{
			fp.direction.x = cos(glm::radians(fp.pitch)) * cos(glm::radians(fp.yaw));
			fp.direction.y = sin(glm::radians(fp.pitch));
			fp.direction.z = cos(glm::radians(fp.pitch)) * sin(glm::radians(fp.yaw));
			fp.direction = noz(fp.direction);

			float y = 0.f;
			float x = 0.f;
			if(ki.up) {
				y += 1;
			}
			if(ki.down) {
				y -= 1;
			}
			if(ki.right) {
				x += 1;
			}
			if(ki.left) {
				x -= 1;
			}

			v2 input = noz(v2(x, y));

			fp.position += fp.direction * input.y * fp.speed * deltaTime;
			fp.position += glm::cross(fp.direction, v3(0,1,0)) * input.x * fp.speed * deltaTime;
			fp.position.y = fp.eyeHeight;

			v3* pos = &fp.position;
			// printf("pitch %f, yaw %f, pos.x %f, pos.y %f, pos.z %f\r", fp.pitch, fp.yaw, pos->x, pos->y, pos->z);
		}

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m4 projection = glm::perspective(glm::radians(90.0f),
			(f32) SCREEN_WIDTH / (f32) SCREEN_HEIGHT, 0.1f, 100.0f);

		m4 view = glm::lookAt(
			fp.position,
			fp.position + fp.direction,
			v3(0,1,0)
		);

		m4 model = glm::translate(m4(1.0f), v3(0,0,0));
		m4 mvp = projection * view * model;

		u32 mvpHandle = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, glm::value_ptr(mvp));

		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, arrayCount(vertexBufferCube));
		glDisableVertexAttribArray(0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		#if 0
		printf("deltatime: %f\n", deltaTime);
		for (i32 v = 0; v < 12*3; v++) {
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
