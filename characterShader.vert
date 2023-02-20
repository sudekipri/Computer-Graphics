#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;
uniform mat4 char_model;
uniform mat4 view;
uniform mat4 char_projection;


void main()
{
    gl_Position = char_projection * view * char_model * vec4(aPos, 1.2f);
    ourColor = aColor;
}