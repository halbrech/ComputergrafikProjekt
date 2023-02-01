#version 430 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

layout (location = 1) uniform sampler2D ourTexture;

void main()
{
    float ambient = 0.1f;
    float diff = max(dot(vec3(1.0f, 1.0f, 0.0f), ourColor), 0.0f);
    FragColor =  (diff + ambient) * texture(ourTexture, TexCoord);
}