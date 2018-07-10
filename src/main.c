#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdarg.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#include "SDL.h"
#pragma clang diagnostic pop

#include "GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>

#define array_count(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

#include "types.h"
#include "math.c"
#include "file.c"
#include "objects.h"

#include <assert.h>

//TODO: Figure out how I want to do logging.
// Not too sure about this current style of logging.

//TODO: Figure out the right length for the buffers in these
// log message functions. 1024 might be a bit of an overkill.
#define buffer_size 1024
static void
log_debug_message(const char* format, ...) {
#ifndef NDEBUG
    char buffer[buffer_size];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, buffer_size, format, args);
    SDL_Log(buffer);
    va_end(args);
#endif
}

static void
log_error_message(const char* format, ...) {
    char buffer[buffer_size];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, buffer_size, format, args);
    SDL_Log(buffer);
    va_end(args);
}
#undef buffer_size

static u32
compile_shader(const char* vertex_shader_code, const char* fragment_shader_code) {
    u32 vertex_shader_id   = glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    i32 result = 0;
    i32 info_log_length;

    // Compile Vertex Shader
    glShaderSource(vertex_shader_id, 1, &vertex_shader_code, NULL);
    glCompileShader(vertex_shader_id);

    // Check Vertex Shader
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0) {
        char error_message_buffer[info_log_length];
        glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, error_message_buffer);
        log_error_message("%s\n", error_message_buffer);
    }

    // Compile Fragment Shader
    glShaderSource(fragment_shader_id, 1, &fragment_shader_code, NULL);
    glCompileShader(fragment_shader_id);

    // Check Fragment Shader
    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0) {
        char error_message_buffer[info_log_length];
        glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, error_message_buffer);
        log_error_message("%s\n", error_message_buffer);
    }

    // Link the program
    u32 program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    // Check the program
    glGetProgramiv(program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0) {
        char error_message_buffer[info_log_length];
        glGetProgramInfoLog(program_id, info_log_length, NULL, error_message_buffer);
        log_error_message("%s\n", error_message_buffer);
    }

    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}

static u32
load_and_compile_shader(const char* vertex_shader_path, const char* fragment_shader_path) {
    u32 program = 0;

    char* vertex_shader_source = 0;
    char* fragment_shader_source = 0;
    if(!read_text_file_contents(vertex_shader_path, &vertex_shader_source)) {
        free_file_contents(vertex_shader_source);
        return 0;
    }
    if(!read_text_file_contents(fragment_shader_path, &fragment_shader_source)) {
        free_file_contents(fragment_shader_source);
        return 0;
    }

    program = compile_shader(vertex_shader_source, fragment_shader_source);

    free_file_contents(vertex_shader_source);
    free_file_contents(fragment_shader_source);

    return program;
}

static u32
load_texture(const char* filename, b32 flip_vertically_on_load, GLint internal_format, GLenum format) {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    i32 width, height, nr_channels;
    stbi_set_flip_vertically_on_load(flip_vertically_on_load);
    u8* data = stbi_load(filename, &width, &height, &nr_channels, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error_message("Failed to load texture\n");
        return 0;
    }
    stbi_image_free(data);
    return texture;
}

static inline u32
load_texture_rgb(const char* filename, b32 flip_vertically_on_load) {
    u32 texture = load_texture(filename, flip_vertically_on_load, GL_RGB, GL_RGB);
    return texture;
}

static inline u32
load_texture_rgba(const char* filename, b32 flip_vertically_on_load) {
    u32 texture = load_texture(filename, flip_vertically_on_load, GL_RGBA, GL_RGBA);
    return texture;
}

static inline void
set_uniform_mat4(const char* name, Mat4 matrix) {
    i32 shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader);

    u32 handle = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(handle, 1, GL_FALSE, &matrix.Elements[0][0]);
}

static inline void
set_uniform_3f(const char* name, f32 f1, f32 f2, f32 f3) {
    i32 shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader);

    u32 handle = glGetUniformLocation(shader, name);
    glUniform3f(handle, f1, f2, f3);
}

static inline void
set_uniform_vec3(const char* name, Vec3 v) {
    set_uniform_3f(name, v.x, v.y, v.z);
}

static inline void
set_unfirorm_1i(const char* name, i32 val) {
    i32 shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
    glUniform1i(glGetUniformLocation(shader, name), val);
}

static inline void
resize_view(RenderContext* render_context, u32 width, u32 height) {
    render_context->width = width;
    render_context->height = height;
    glViewport(0, 0, width, height);
}

static void
run_tests() {
    MeshData mesh_data;
    load_mesh_data_from_obj("data\\nanosuit\\nanosuit.obj", &mesh_data);
}

i32
main() {
    // Init SDL stuff
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        log_error_message("SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

    //TODO: Have our own INTERNAL flag or something
#ifndef NDEBUG 
    run_tests();
#endif

    RenderContext render_context;
    render_context.width = 512;
    render_context.height = 512;

    SDL_Window* window = SDL_CreateWindow(
        "GL_TEST",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        render_context.width, render_context.height,
        SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);

    if(!window) {
        log_error_message("SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // SDL_SetRelativeMouseMode(SDL_TRUE);

    // Init OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    render_context.gl_context = gl_context;
    render_context.window = window;

    SDL_GL_SetSwapInterval(0);

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Load shaders
    u32 basic_shader = load_and_compile_shader("data\\shaders\\basic_vertex.glsl",
                                               "data\\shaders\\basic_fragment.glsl");
    u32 light_shader = load_and_compile_shader("data\\shaders\\basic_vertex.glsl",
                                               "data\\shaders\\light_fragment.glsl");
    if(!basic_shader ||
       !light_shader) {
        log_error_message("Error loading shaders.\n");
        return -1;
    }

//Load textures
#define USE_TEXTURES 0
#if USE_TEXTURES
    u32 texture0 = load_texture_rgb("data/textures/container.jpg", true);
    u32 texture1 = load_texture_rgba("data/textures/awesomeface.png", true);
    if(!texture0 ||
       !texture1) {
        debugLog("Error loading textures.\n");
        return -1;
    }
#endif

    Vec3 cube_positions[] = {
        vec3( 0.0f,  0.0f,  0.0f),
        vec3( 2.0f,  5.0f, -15.0f),
        vec3(-1.5f, -2.2f, -2.5f),
        vec3(-3.8f, -2.0f, -12.3f),
        vec3( 2.4f, -0.4f, -3.5f),
        vec3(-1.7f,  3.0f, -7.5f),
        vec3( 1.3f, -2.0f, -2.5f),
        vec3( 1.5f,  2.0f, -2.5f),
        vec3( 1.5f,  0.2f, -1.5f),
        vec3(-1.3f,  1.0f, -1.5f),
    };
    Rotation cube_rotations[] = {
        { .axis = vec3(0, 1, 0), .angle = 30 },
        { .axis = vec3(1, 1, 1), .angle = 24 },
        { .axis = vec3(0, 1, 1), .angle = 30 },
        { .axis = vec3(1, 1, 0), .angle = 60 },
        { .axis = vec3(1, 1, 0), .angle = 60 },
        { .axis = vec3(0, 1, 1), .angle = 30 },
        { .axis = vec3(1, 1, 1), .angle = 24 },
        { .axis = vec3(0, 1, 1), .angle = 30 },
        { .axis = vec3(0, 1, 1), .angle = 30 },
        { .axis = vec3(1, 0, 1), .angle = 10 },
    };

    i32 cube_count = array_count(cube_positions);
    Mesh cube_mesh_array[array_count(cube_positions)];

    u32 cube_vertex_array;
    glGenVertexArrays(1, &cube_vertex_array);

    u32 cube_vertex_buffer;
    glGenBuffers(1, &cube_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_positions), cube_vertex_positions, GL_STATIC_DRAW);

    u32 cube_normal_buffer;
    glGenBuffers(1, &cube_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);

    // u32 cube_color_buffer;
    // glGenBuffers(1, &cube_color_buffer);
    // u32 cubeTexCoordBuffer;
    // glGenBuffers(1, &cubeTexCoordBuffer);

    //Make cube vertex array
    glBindVertexArray(cube_vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, cube_normal_buffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, cube_color_buffer);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexColors), cubeVertexColors, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, cubeTexCoordBuffer);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexCoords), cubeTexCoords, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(2);

    //Make light vertex array
    u32 light_vertex_array;
    glGenVertexArrays(1, &light_vertex_array);

    glBindVertexArray(light_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //Cube transform data;
    const int cube_triangle_count = array_count(cube_vertex_positions) / 3;

    for(i32 i = 0; i < cube_count; ++i) {
        cube_mesh_array[i].vao           = cube_vertex_array;
        cube_mesh_array[i].count         = cube_triangle_count;
        cube_mesh_array[i].shader_program = basic_shader;
    }

#if USE_TEXTURES
    glUseProgram(basic_shader);
    set_unfirorm1i("in_texture_0", 0);
    set_unfirorm1i("in_texture_1", 1);
    // glUniform1i(glGetUniformLocation(basic_shader, "in_texture0"), 0);
    // glUniform1i(glGetUniformLocation(basic_shader, "in_texture1"), 1);
#endif

    Vec3 light_pos = { .x = 1.2f, .y = 1.0f, .z = 2.0f};
    Vec3 view_pos  = { .x = 0.0f, .y = 2.0f, .z = 3.0f};

    glUseProgram(basic_shader);
    set_uniform_3f("object_color", 1.0f, 0.5f, 0.31f);
    set_uniform_3f("light_color", 1.0f, 1.0f, 1.0f);
    set_uniform_vec3("light_pos", light_pos);
    set_uniform_vec3("view_pos", view_pos);

    b32 running = true;
    f64 current_time = (f32)SDL_GetPerformanceCounter() /
                      (f32)SDL_GetPerformanceFrequency();
    f64 last_time = 0;
    f64 delta_time = 0;
    i32 frame_counter = 0;
    i32 last_frame_count = 0;
    f64 last_fps_time = 0;
    while(running) {
        last_time = current_time;
        current_time = (f64)SDL_GetPerformanceCounter() /
                      (f64)SDL_GetPerformanceFrequency();
        delta_time = (f64)(current_time - last_time);

        // Count frames for every second and print it as the title of the window
        ++frame_counter;
        if(current_time >= (last_fps_time + 1.f)) {
            last_fps_time    = current_time;
            i32 delta_frames = frame_counter - last_frame_count;
            last_frame_count = frame_counter;
            char title[64];
            sprintf(title, "FPS: %d", delta_frames);
            SDL_SetWindowTitle(window, title);
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: {
                    running = false;
                } break;

                case SDL_WINDOWEVENT: {
                    switch(event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            resize_view(&render_context, event.window.data1, event.window.data2);
                        } break;
                    }
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

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 projection = HMM_Perspective(90.f, (f32)render_context.width / (f32)render_context.height, 0.1f, 1000.f);

        Mat4 view = HMM_LookAt(
            view_pos,
            vec3(0, 0, 0),
            vec3(0, 1, 0)
        );

// Rotate cubes
#if 1
        for(i32 i = 0; i < cube_count; i++) {
            float rotation_amount   = delta_time * 100;
            cube_rotations[i].angle = fmod(cube_rotations[i].angle + rotation_amount, 360);
        }
#endif

// Draw light source
#if 1
        {
            Mat4 model = HMM_Mat4d(1.0f);
            model = HMM_MultiplyMat4(model, HMM_Translate(light_pos));
            model = HMM_MultiplyMat4(model, HMM_Scale(vec3(0.3f, 0.3f, 0.3f)));

            glUseProgram(light_shader);

            set_uniform_mat4("model", model);
            set_uniform_mat4("view", view);
            set_uniform_mat4("projection", projection);

            glBindVertexArray(light_vertex_array);
            glDrawArrays(GL_TRIANGLES, 0, cube_triangle_count);
        }
#endif

// Draw cubes
#if 1
        for(i32 i = 0; i < cube_count; i++) {
            // Vec3 scale = cubeScales[i];
            Vec3 scale = vec3(1.0f, 1.0f, 1.0f);
            Vec3 position = cube_positions[i];
            Mesh mesh = cube_mesh_array[i];
            Rotation rotation = cube_rotations[i];

            Mat4 model = HMM_Mat4d(1.0f);
            model = HMM_MultiplyMat4(model, HMM_Translate(position));
            model = HMM_MultiplyMat4(model, HMM_Rotate(rotation.angle, rotation.axis));
            model = HMM_MultiplyMat4(model, HMM_Scale(scale));
            // Mat4 mvp = projection * view * model;

            glUseProgram(mesh.shader_program);

            set_uniform_mat4("model", model);
            set_uniform_mat4("view", view);
            set_uniform_mat4("projection", projection);

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

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    return 0;
}
