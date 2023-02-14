#version 430 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform vec4 lightdir;

layout (location = 1) uniform sampler2D ourTexture;

void main()
{
    float ambient = 0.1f;
    float diff = max(dot(lightdir.xyz, ourColor), 0.0f);
    FragColor =  (diff + ambient) * texture(ourTexture, TexCoord);
}