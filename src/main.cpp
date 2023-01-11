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
    GLuint VERTEX_BUFFER_OBJECT;
    GLuint VERTEX_ARRAY_OBJECT;
    Mesh(std::vector<Vertex> &vertices) : vertices(vertices), VERTEX_BUFFER_OBJECT(0), VERTEX_ARRAY_OBJECT(0){};
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
 
int main(void)
{

    //Creating a window
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
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
    vpos_location = glGetAttribLocation(program, "vPos");
    vnorm_location = glGetAttribLocation(program, "vNorm");
    vcol_location = glGetAttribLocation(program, "vCol");
    vuv_location = glGetAttribLocation(program, "InTexCoord");
    texture_location = glGetAttribLocation(program, "Texture");

    //dummy
    # define PI 3.14159265358979323846f
    Sphere sphere(3);
    std::vector<Vertex> vertices;
    for(auto v : sphere.vertices) {
        Vertex vertex = {.pos = v, .norm = v, .color = v, .uv = glm::vec2(asin(v.x)/PI + 0.5f, asin(v.y)/PI + 0.5f)};
        vertices.push_back(vertex);
    }
    Mesh mesh(vertices);

    //Buffers
    glGenBuffers(1, &VboID);
    glBindBuffer(GL_ARRAY_BUFFER, VboID);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VaoID);
    glBindVertexArray(VaoID);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (0 * sizeof(float)));  // 3 floats für Position
    glEnableVertexAttribArray(vnorm_location);
    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (3 * sizeof(float))); // 3 floats für den Normalenvektor
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (6 * sizeof(float))); // 3 floats für den Farbwert
    glEnableVertexAttribArray(vuv_location);
    glVertexAttribPointer(vuv_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (9 * sizeof(float))); // 2 floats als Textur-Koordinaten

    glGenBuffers(1, &IboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(glm::uvec3), sphere.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(VaoID);

    //get png
    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load("assets/test.png", &w, &h, &comp, STBI_rgb_alpha);

    glGenTextures(1, &texture_location);
    glBindTexture(GL_TEXTURE_2D, texture_location);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, texture_location);
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
        mat4x4_rotate_X(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
 
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glDrawArrays(GL_TRIANGLES, 0, 60);
        // TODO glDrawElements
        glDrawElements(GL_TRIANGLES, 9 * sphere.indices.size(), GL_UNSIGNED_INT, NULL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}