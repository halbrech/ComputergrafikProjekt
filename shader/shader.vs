#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorms;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

layout (location = 0) uniform mat4 MVP;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}