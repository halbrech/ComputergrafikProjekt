#version 110
uniform sampler2D Texture;
varying vec3 color;
varying vec2 TexCoord;
void main()
{
    //gl_FragColor = vec4(color, 1.0);
    gl_FragColor = texture2D(Texture, TexCoord);
};