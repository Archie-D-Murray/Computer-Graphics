#version 330 core

// Outputs
out vec3 colour;

// Uniforms
uniform vec3 lightColour;

void main ()
{
    colour = lightColour;
}
