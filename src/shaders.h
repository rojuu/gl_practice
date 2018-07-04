/*
  TODO:
    - Should we actually switch back to having these in separate text files?
*/

const char *basic_fragment_shader = R"FOO(
#version 330 core
out vec4 color;
/*
in vec3 vertex_color;
in vec2 tex_coord;
*/

/*
uniform sampler2D in_texture_0;
uniform sampler3D in_texture_1;
*/

uniform vec3 object_color;

uniform vec3 light_color;
uniform vec3 light_pos;

uniform vec3 view_pos;

in vec3 frag_pos;
in vec3 normal;

void main() {
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);

    float diff = max(dot(norm, light_dir), 0);
    vec3 diffuse = diff * light_color;

    float specular_strength = 0.5;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * object_color;
    color = vec4(result, 1.0);
}
)FOO";

const char *basic_vertex_shader = R"FOO(
#version 330 core
layout(location = 0) in vec3 in_vertex_position;
layout(location = 1) in vec3 in_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 frag_pos;
out vec3 normal;

void main(){
    gl_Position = projection * view * model * vec4(in_vertex_position, 1.0f);
    frag_pos = vec3(model * vec4(in_vertex_position, 1.0));
    normal = mat3(transpose(inverse(model))) * in_normal;
}
)FOO";

const char *light_fragment_shader = R"FOO(
#version 330 core
out vec4 color;

void main() {
    color = vec4(1.0f);
}
)FOO";
