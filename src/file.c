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

static char*
find_next_non_whitespace_in_string(char* str) {
    i32 counter = 0;
    while(*str == ' ' || *str == '\t') {
        str++;
        counter++;
        if(counter < 0) {
            //We probably got some stupid data here, because we wrapped the int
            return 0;
        }
    }
    return str;
}

static char*
find_next_digit_in_string(char* str) {
    i32 counter = 0;
    while(!isdigit(*str)) {
        str++;
        counter++;
        if(counter < 0) {
            //We probably got some stupid data here, because we wrapped the int
            return 0;
        }
    }
    return str;
}

static char*
find_next_non_digit_in_string(char* str) {
    i32 counter = 0;
    while(isdigit(*str)) {
        str++;
        counter++;
        if(counter < 0) {
            //We probably got some stupid data here, because we wrapped the int
            return 0;
        }
    }
    return str;
}

static Vec3i
obj_parse_single_index(char* file_contents, char** out_end_ptr) {
    Vec3i result = {};
    char* str_ptr = file_contents;

    str_ptr = find_next_digit_in_string(str_ptr);
    result.x = atoi(str_ptr);

    str_ptr = find_next_non_digit_in_string(str_ptr);
    str_ptr++;
    if(*str_ptr != '/')
        result.y = atoi(str_ptr);
    
    str_ptr = find_next_non_digit_in_string(str_ptr);
    str_ptr++;
    if(*str_ptr != '/')
        result.z = atoi(str_ptr);

    while(isdigit(*str_ptr)) {
        str_ptr++;
    }

    *out_end_ptr = str_ptr;

    return result;
}

static char*
obj_parse_indices(char* file_contents, Vec3i* vertex_i, Vec3i* tex_coord_i, Vec3i* normal_i) {
    if(!file_contents) {
        return file_contents;
    }

    if(!vertex_i || !normal_i || !tex_coord_i) {
        return file_contents;
    }

    *vertex_i    = obj_parse_single_index(file_contents, &file_contents);
    *tex_coord_i = obj_parse_single_index(file_contents, &file_contents);
    *normal_i    = obj_parse_single_index(file_contents, &file_contents);

    return file_contents;
}

static void write_mesh_data_to_file(const char* filename, MeshData* mesh_data);

//TODO: Properly test this with some edge cases
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
        .vertices          = malloc(line_count * sizeof(Vec3)),
        .normals           = malloc(line_count * sizeof(Vec3)),
        .tex_coords        = malloc(line_count * sizeof(Vec2)),
        .vertex_indices    = malloc(line_count * sizeof(Vec3i)),
        .tex_coord_indices = malloc(line_count * sizeof(Vec3i)),
        .normal_indices    = malloc(line_count * sizeof(Vec3i)),
    };

    u32 vertex_count = 0;
    u32 normal_count = 0;
    u32 tex_coord_count = 0;
    u32 index_count = 0;

    char* fptr = file_contents;
    for(;;) {
        switch(*fptr) {
            //INCOMPLETE: We are not handling all possible obj cases.
            // Skip any lines starting with these
            case 'm': // mtllib
            case 'u': // usemtl
            case 's': // smooth shading
            case 'o': // object name
            case 'l': // line segments
            case 'g': // group name
            case '#': {
                fptr = find_next_line_in_string(fptr);
                continue;
            } break;

            // Skip whitespace
            case '\t':
            case ' ': {
                fptr++;
                continue;
            } break;

            // Skip newlines (lf or crlf should both be handled, not tested though)
            case '\r': {
                if(*(fptr+1) == '\n') {
                    fptr+=2;
                } else {
                    fptr++;
                }
                continue;
            } break;
            case '\n': {
                fptr++;
                continue;
            } break;

            // Parse vertex data (positions,tex_coords,normals)
            case 'v': {
                fptr++;
                switch(*fptr) {
                    case ' ': {
                        Vec3 vertex = obj_parse_vertex(fptr);
                        mesh_data.vertices[vertex_count++] = vertex;
                        fptr = find_next_line_in_string(fptr);
                    } break;

                    case 't': {
                        Vec2 tex_coord = obj_parse_tex_coord(fptr);
                        mesh_data.tex_coords[tex_coord_count++] = tex_coord;
                        fptr = find_next_line_in_string(fptr);
                    } break;

                    case 'n': {
                        Vec3 normal = obj_parse_normal(fptr);
                        mesh_data.normals[normal_count++] = normal;
                        fptr = find_next_line_in_string(fptr);
                    } break;
                }
            } break;

            // Parse indices/faces
            case 'f': {
                fptr = obj_parse_indices(fptr,
                                         &mesh_data.vertex_indices[index_count],
                                         &mesh_data.tex_coord_indices[index_count],
                                         &mesh_data.normal_indices[index_count]);
                index_count++;
                fptr = find_next_line_in_string(fptr);
            } break;

            default:
                assert(!"load_mesh_data_from_obj unhandled case, should never happen");
                break;
        }

        if(*fptr == '\0') {
            break;
        }
    }

    mesh_data.vertices_count = vertex_count;
    mesh_data.normals_count = normal_count;
    mesh_data.tex_coords_count = tex_coord_count;
    mesh_data.indices_count = index_count;

    free_file_contents(file_contents);

    *out_mesh_data = mesh_data;

    write_mesh_data_to_file("data\\nanosuit\\nanosuit_remade.obj", &mesh_data);
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

static void
write_mesh_data_to_file(const char* filename, MeshData* mesh_data) {
    char buffer[1024];

    FILE* file_handle;
    if(fopen_s(&file_handle, filename, "w")) {
        return;
    }

    u32 i;
    for(i = 0; i < mesh_data->vertices_count; i++) {
        Vec3 vertex = mesh_data->vertices[i];
        if(sprintf_s(buffer, 1024, "v %f %f %f\n", vertex.x, vertex.y, vertex.z) < 0) {
            assert(!"write_mesh_data_to_file: sprintf_s error");
        }

        if(fputs(buffer, file_handle) < 0) {
            assert(!"write_mesh_data_to_file: fputs error");
        }
    }

    for(i = 0; i < mesh_data->normals_count; i++) {
        Vec3 normal = mesh_data->normals[i];
        if(sprintf_s(buffer, 1024, "vn %f %f %f\n", normal.x, normal.y, normal.z) < 0) {
            assert(!"write_mesh_data_to_file: sprintf_s error");
        }

        if(fputs(buffer, file_handle) < 0) {
            assert(!"write_mesh_data_to_file: fputs error");
        }
    }

    for(i = 0; i < mesh_data->tex_coords_count; i++) {
        Vec2 tex_coord = mesh_data->tex_coords[i];
        if(sprintf_s(buffer, 1024, "vt %f %f\n", tex_coord.x, tex_coord.y) < 0) {
            assert(!"write_mesh_data_to_file: sprintf_s error");
        }

        if(fputs(buffer, file_handle) < 0) {
            assert(!"write_mesh_data_to_file: fputs error");
        }
    }

    for(i = 0; i < mesh_data->indices_count; i++) {
        Vec3i vertex_index = mesh_data->vertex_indices[i];
        Vec3i tex_coord_index = mesh_data->tex_coord_indices[i];
        Vec3i normal_index = mesh_data->normal_indices[i];

        if(sprintf_s(buffer, 1024, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                     vertex_index.x, vertex_index.y, vertex_index.z,
                     tex_coord_index.x, tex_coord_index.y, tex_coord_index.z,
                     normal_index.x, normal_index.y, normal_index.z)
                        < 0)
        {
            assert(!"write_mesh_data_to_file: sprintf_s error");
        }

        if(fputs(buffer, file_handle) < 0) {
            assert(!"write_mesh_data_to_file: fputs error");
        }
    }
}
