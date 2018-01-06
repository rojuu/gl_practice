#version 330 core
out vec4 color;
in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D inTexture0;
uniform sampler2D inTexture1;

void main() {
    vec4 color0 = texture(inTexture0, texCoord);
    vec4 color1 = texture(inTexture1, texCoord);
    color = mix(color0, color1, color1.a * 0.2);
}
