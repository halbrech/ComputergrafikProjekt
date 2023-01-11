#version 430 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

layout (location = 1) uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
    //FragColor = vec4(TexCoord, 1.0, 1.0);
}