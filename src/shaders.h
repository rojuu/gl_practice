/*
  TODO: 
    - Should we actually switch back to having these in separate text files?
*/

const char *basic_fragment_shader = R"FOO(
#version 330 core
out vec4 color;
/*
in vec3 vertexColor;
in vec2 texCoord;
*/

/*
uniform sampler2D inTexture0;
uniform sampler3D inTexture1;
*/

uniform vec3 objectColor;

uniform vec3 lightColor;
uniform vec3 lightPos;

uniform vec3 viewPos;

in vec3 fragPos;
in vec3 normal;

void main() {
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 1.0);
}
)FOO";

const char *basic_vertex_shader = R"FOO(
#version 330 core
layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 normal;

void main(){
    gl_Position = projection * view * model * vec4(inVertexPosition, 1.0f);
    fragPos = vec3(model * vec4(inVertexPosition, 1.0));
    normal = mat3(transpose(inverse(model))) * inNormal;
}
)FOO";

const char *light_fragment_shader = R"FOO(
#version 330 core
out vec4 color;

void main() {
    color = vec4(1.0f);
}
)FOO";
