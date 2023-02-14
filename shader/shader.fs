#version 430 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform vec4 lightdir;

layout (location = 0) uniform sampler2D ourTexture;
layout (location = 1) uniform sampler2D ourTexture2;

void main()
{
    float ambient = 0.1f;
    float diff = max(dot(lightdir.xyz, ourColor), 0.0f);
    FragColor = mix(vec4(texture(ourTexture2, TexCoord).x), texture(ourTexture, TexCoord), diff + 0.1);
}