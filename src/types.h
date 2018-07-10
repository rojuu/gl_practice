typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

#define true 1
#define false 0

typedef hmm_vec2 Vec2;
typedef hmm_vec3 Vec3;
typedef hmm_vec4 Vec4;

typedef hmm_mat4 Mat4;

typedef struct {
    SDL_Window* window;
    SDL_GLContext gl_context;
    u32 width;
    u32 height;
} RenderContext;

//TODO: Make this a bitmask
typedef struct {
    b32 left;
    b32 right;
    b32 up;
    b32 down;
} KeyboardInput;

typedef struct {
    Vec3* vertices;
    Vec3* normals;
    Vec2* tex_coords;
    i32* indices;
    u32 count;
} MeshData;

typedef struct {
    u32 vao;
    u32 count;
    u32 shader_program;
} Mesh;

typedef struct {
    Vec3 axis;
    f32 angle;
} Rotation;