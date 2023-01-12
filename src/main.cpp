#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include "sphere.cpp"

#include <linmath/linmath.h>
#include <vector>
 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec3 color;
    glm::vec2 uv;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<glm::uvec3> indices;
    GLuint VERTEX_ARRAY_OBJECT;
    GLuint VERTEX_BUFFER_OBJECT;
    GLuint INDEX_BUFFER_OBJECT;
    Mesh(std::vector<Vertex> &vertices, std::vector<glm::uvec3> &indices) : vertices(vertices), indices(indices) {
        // Vao
        glGenVertexArrays(1, &VERTEX_ARRAY_OBJECT);
        glBindVertexArray(VERTEX_ARRAY_OBJECT);
        // Vbo
        glGenBuffers(1, &VERTEX_BUFFER_OBJECT);
        glBindBuffer(GL_ARRAY_BUFFER, VERTEX_BUFFER_OBJECT);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (0 * sizeof(float)));  // 3 floats für Position
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (3 * sizeof(float))); // 3 floats für den Normalenvektor
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (6 * sizeof(float))); // 3 floats für den Farbwert
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (9 * sizeof(float))); // 2 floats als Textur-Koordinaten
        // Ibo
        glGenBuffers(1, &INDEX_BUFFER_OBJECT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, INDEX_BUFFER_OBJECT);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);
    }

    Mesh(std::vector<glm::vec3> &pos, std::vector<glm::vec3> &norm, std::vector<glm::vec3> &color, std::vector<glm::vec2> &texturecoor, std::vector<glm::uvec3> &indices) {
        std::vector<Vertex> vertices;
        vertices.reserve(pos.size());
        for (size_t i = 0; i < pos.size(); i++) {
            Vertex v = {.pos = pos[i], .norm = norm[i], .color = color[i], .uv = texturecoor[i]};
            vertices.push_back(v);
        }
        Mesh(vertices, indices);
    }

    void draw(GLuint shader);
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLAPIENTRY DebugMessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

int main(void)
{

    //Creating a window
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
 
    GLuint VboID, VaoID, IboID, vertex_shader, fragment_shader, program, texture_location;
    GLint mvp_location, vpos_location, vnorm_location, vcol_location, vuv_location;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugMessageCallback, 0);

    //Shaders
    const char* vertexPath = "shader/shader.vs";
    const char* fragmentPath = "shader/shader.fs";
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();


    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vShaderCode, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fShaderCode, NULL);
    glCompileShader(fragment_shader);
 
    //Program
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    texture_location = glGetUniformLocation(program, "ourTexture");

    //dummy
    Sphere sphere(3);
    auto texturecoor = sphere.getTextureCoor();
    Mesh earth(sphere.vertices, sphere.vertices, sphere.vertices, texturecoor, sphere.indices);
    
    
    //get png
    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load("assets/test.png", &w, &h, &comp, 0);
    std::cout << w << ", " << h << std::endl;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
 
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 
        mat4x4_identity(m);
        mat4x4_rotate_Y(m, m, (float) glfwGetTime()/100);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
 
        glUseProgram(program);
        glUniformMatrix4fv(0, 1, GL_FALSE, (const GLfloat*) mvp);
        //glUniform1i(1, 0);
        glDrawElements(GL_TRIANGLES, 3 * sphere.indices.size(), GL_UNSIGNED_INT, NULL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}