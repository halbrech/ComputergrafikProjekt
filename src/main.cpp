#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "sphere.cpp"
 
#include <linmath/linmath.h>
#include <vector>
 
#include <stdlib.h>
#include <stdio.h>

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";
 
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
 
    GLuint VboID, VaoID, IboID, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vnorm_location, vcol_location, vuv_location;

    glEnable(GL_DEPTH_TEST);

    //Shaders
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
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
    vuv_location = glGetAttribLocation(program, "vUV");

    //dummy
    Sphere sphere{};
    std::vector<Vertex> vertices;
    for(auto v : sphere.vertices) {
        Vertex vertex = {.pos = v, .norm = v, .color = glm::vec3(1.f, 1.f, 1.f), .uv = glm::vec2(0.f, 0.f)};
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
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);  // 3 floats für Position
    glEnableVertexAttribArray(vnorm_location);
    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0); // 3 floats für den Normalenvektor
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0); // 3 floats für den Normalenvektor
    glEnableVertexAttribArray(vuv_location);
    glVertexAttribPointer(vuv_location, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*) 0); // 2 floats als Textur-Koordinaten

    glGenBuffers(1, &IboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(glm::uvec3), sphere.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(VaoID);

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
        //glDrawElements(GL_TRIANGLES, 3 * sphere.indices.size(), GL_UNSIGNED_INT, NULL); ????
        glDrawElements(GL_TRIANGLES, 9 * sphere.indices.size(), GL_UNSIGNED_INT, NULL);
        //glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        //glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}