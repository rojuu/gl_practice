#version 330 core
layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec3 inVertexColor;
layout(location = 2) in vec2 inTexCoord;

uniform mat4 MVP;
out vec3 vertexColor;
out vec2 texCoord;

void main(){
    gl_Position =  MVP * vec4(inVertexPosition, 1);
    vertexColor = inVertexColor;
    texCoord = inTexCoord;
}
