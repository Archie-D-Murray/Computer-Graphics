#version 330 core

// Inputs
layout(location = 0) in vec3 position;

// Uniforms
uniform mat4 MVP;

void main()
{
    // Output vertex postion
    gl_Position = MVP * vec4(position, 1.0);
}
