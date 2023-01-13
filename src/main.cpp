#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader/shader.h>
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugMessageCallback, 0);

    //shader
    Shader earthShader("shader/shader.vs", "shader/shader.fs");

    //earth
    Sphere sphere(3);
    auto texturecoor = sphere.getTextureCoor();
    Mesh earth(sphere.vertices, sphere.vertices, sphere.vertices, texturecoor, sphere.indices);
    
    //create texture
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

    //create matrix
    int SCR_WIDTH = 800;
    int SCR_HEIGHT = 600;
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0,0,-3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //configure shader
    earthShader.use();
    earthShader.setInt("ourTexture", 0);
    earthShader.setMat4("model", model);
    earthShader.setMat4("view", view);
    earthShader.setMat4("projection", projection);

    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 3 * sphere.indices.size(), GL_UNSIGNED_INT, NULL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}