/*
 * Returns number of bytes read.
 * file_contents is guaranteed to be null-terminated.
 */
static u32
read_text_file_contents(const char* filename, char** file_contents) {
    u32 number_of_bytes_read = 0;
    HANDLE file_handle = CreateFileA(filename,
                                     GENERIC_READ,
                                     0, 0,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);

    if(file_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    LARGE_INTEGER large_integer;
    if(!GetFileSizeEx(file_handle, &large_integer)) {
        return 0;
    }

    u32 number_of_bytes = (u32)large_integer.QuadPart;

    char* buffer = calloc(number_of_bytes + 1, 1);
    if(!ReadFile(file_handle, buffer, number_of_bytes, (LPDWORD)&number_of_bytes_read, 0)) {
        return 0;
    }

    buffer[number_of_bytes] = '\0';

    *file_contents = buffer;

    CloseHandle(file_handle);

    return number_of_bytes;
}

static inline void
free_file_contents(char* file_contents) {
    free(file_contents);
}

static u32
count_lines_in_file(char* file_contents) {
    u32 line_count = 0;
    for(;;) {
        if(*file_contents == '\0') break;
        if(*file_contents == '\n') {
            line_count++;
        }
        file_contents++;
    }

    return line_count;
}

static char *
find_next_line_in_string(char* str_ptr) {
    if(str_ptr == 0) return str_ptr;
    while(*str_ptr != '\0' && *str_ptr != '\n') {
        str_ptr++;
    }

    if(*str_ptr != '\0') str_ptr++;

    return str_ptr;
}

static Vec3
obj_parse_vertex(char* file_contents) {
    Vec3 result = {};

    if(!file_contents) {
        return result;
    }

    while(!isdigit(*file_contents) && *file_contents != '\0') {
        file_contents++;
    }

    if(isdigit(*file_contents)) {
        char* str_end;
        result.x = strtof(file_contents, &str_end);
        file_contents = str_end;

        result.y = strtof(file_contents, &str_end);
        file_contents = str_end;

        result.z = strtof(file_contents, 0);
    }

    return result;
}

static Vec2
obj_parse_tex_coord(char* file_contents) {
    Vec2 result = {};

    if(!file_contents) {
        return result;
    }

    while(!isdigit(*file_contents) && *file_contents != '\0') {
        file_contents++;
    }

    if(isdigit(*file_contents)) {
        char* str_end;
        result.x = strtof(file_contents, &str_end);
        file_contents = str_end;

        result.y = strtof(file_contents, 0);
    }

    return result;
}

//COPYPASTE: This is exactly the same as loading a vertex, except we normalize
// the output, because obj normals are not guaranteed to be normalized.
static Vec3
obj_parse_normal(char* file_contents) {
    Vec3 result = {};

    if(!file_contents) {
        return result;
    }

    while(!isdigit(*file_contents) && *file_contents != '\0') {
        file_contents++;
    }

    if(isdigit(*file_contents)) {
        char* str_end;
        result.x = strtof(file_contents, &str_end);
        file_contents = str_end;

        result.y = strtof(file_contents, &str_end);
        file_contents = str_end;

        result.z = strtof(file_contents, 0);
    }

    result = HMM_NormalizeVec3(result);

    return result;
}

//TODO: Faces have up to three values in obj format, for example "1/1/1"
// We are not accounting for that little feature of obj.
static i32
obj_parse_index(char* file_contents) {
    i32 result = 0;

    if(!file_contents) {
        return result;
    }

    while(!isdigit(*file_contents) && *file_contents != '\0') {
        file_contents++;
    }
    if(isdigit(*file_contents)) {
        result = atoi(file_contents);
    }

    return result;
}

static b32
load_mesh_data_from_obj(const char* filename, MeshData* out_mesh_data) {
    char* file_contents;
    if(!read_text_file_contents(filename, &file_contents) || file_contents == 0) {
        free_file_contents(file_contents);
        return 0;
    }

    // We can't have more items than our line_count,
    // because there are max one of each per line in an obj file
    u32 line_count = count_lines_in_file(file_contents);
    MeshData mesh_data = {
        .vertices   = malloc(line_count * sizeof(Vec3)),
        .normals    = malloc(line_count * sizeof(Vec3)),
        .tex_coords = malloc(line_count * sizeof(Vec2)),
        .indices    = malloc(line_count * sizeof(i32)),
    };

    u32 vertex_count = 0;
    u32 normal_count = 0;
    u32 tex_coord_count = 0;
    u32 index_count = 0;

    for(;;) {
        switch(*file_contents) {
            //INCOMPLETE: We are not handling all possible obj cases.
            // Skip any lines starting with these
            case 'm': // mtllib
            case 'u': // usemtl
            case 's': // smooth shading
            case 'o': // object name
            case 'l': // line segments
            case 'g': // group name
            case '#': {
                file_contents = find_next_line_in_string(file_contents);
                continue;
            } break;

            // Skip whitespace
            case '\t':
            case ' ': {
                file_contents++;
                continue;
            } break;

            // Skip newlines (lf or crlf should both be handled)
            case '\r': {
                if( *(file_contents+1) == '\n' ) {
                    file_contents+=2;
                } else {
                    file_contents++;
                }
                continue;
            } break;

            case '\n': {
                file_contents++;
                continue;
            } break;

            // Parse vertex data (positions,tex_coords,normals)
            case 'v': {
                file_contents++;
                switch(*file_contents) {
                    case ' ': {
                        Vec3 vertex = obj_parse_vertex(file_contents);
                        mesh_data.vertices[vertex_count++] = vertex;
                        file_contents = find_next_line_in_string(file_contents);
                    } break;

                    case 't': {
                        Vec2 tex_coord = obj_parse_tex_coord(file_contents);
                        mesh_data.tex_coords[tex_coord_count++] = tex_coord;
                        file_contents = find_next_line_in_string(file_contents);
                    } break;

                    case 'n': {
                        Vec3 normal = obj_parse_normal(file_contents);
                        mesh_data.normals[tex_coord_count++] = normal;
                        file_contents = find_next_line_in_string(file_contents);
                    } break;
                }
            } break;

            // Parse indices/faces
            case 'f': {
                i32 index = obj_parse_index(file_contents);
                mesh_data.indices[index_count++] = index;
                file_contents = find_next_line_in_string(file_contents);
            } break;

            default:
                assert(!"load_mesh_data_from_obj unhandled case, should never happen");
                break;
        }

        if(*file_contents == '\0') {
            break;
        }
    }

    u32 final_count = vertex_count;

    free_file_contents(file_contents);

    *out_mesh_data = mesh_data;
    return 1;
}

static inline Mesh
make_mesh_from_mesh_data(MeshData mesh_data) {
    (void) mesh_data;
    Mesh mesh = {};
    return mesh;
}

static b32
make_mesh_from_obj(const char* filename, Mesh* mesh) {
    (void) filename;
    (void) mesh;
    return 0;
}
