#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColour;

void main() {
    color = vec4(textColour, texture(text, TexCoords).r);
}
