#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec3 vPos;
attribute vec2 InTexCoord;
varying vec3 color;
varying vec2 TexCoord;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
    TexCoord = InTexCoord;
};