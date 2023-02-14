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
#include <chrono>
#include <thread>

# define PI 3.14159265358979323846f

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
    void create(std::vector<Vertex> &vertices) {
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
        glBindVertexArray(0);
    }

    Mesh(std::vector<glm::vec3> &pos, std::vector<glm::vec3> &norm, std::vector<glm::vec3> &color, std::vector<glm::vec2> &texturecoor, std::vector<glm::uvec3> &inindices) {
        indices = inindices;
        vertices.reserve(pos.size());
        for (size_t i = 0; i < pos.size(); i++) {
            Vertex v = {.pos = pos[i], .norm = norm[i], .color = color[i], .uv = texturecoor[i]};
            vertices.push_back(v);
        }
        create(vertices);
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

unsigned int loadTexture(std::string path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void updateTexture(unsigned int textureID, std::string path)
{
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    //stbi_set_flip_vertically_on_load(true);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            std::cout << width << ", " << height << ", " << nrChannels << "\n";
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //stbi_set_flip_vertically_on_load(false);
    return textureID;
}

std::vector<std::string> generateDates(int year) {
  struct tm date = {0, 0, 0, 1, 8, year - 1900}; // Initialize a tm struct with the first day of the year
  std::vector<std::string> dates;

  while (date.tm_year == year - 1900) {
    char dateString[13];
    for (size_t hour = 0; hour < 2400; hour += 100)
    {
        sprintf(dateString, "%04d%02d%02d_%04d", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, hour);
        dates.push_back(dateString);
    }
    // Increment the date by one day
    date.tm_mday++;
    mktime(&date);
  }

  return dates;
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
    Shader moonShader("shader/moon.vs", "shader/moon.fs");
    Shader universeboxShader("shader/universebox.vs", "shader/universebox.fs");

    //earth
    Sphere sphere(3);
    auto texturecoor = sphere.getTextureCoor();
    Mesh earth(sphere.vertices, sphere.vertices, sphere.vertices, texturecoor, sphere.indices);
    Mesh moon(sphere.vertices, sphere.vertices, sphere.vertices, texturecoor, sphere.indices);
    
    //universebox
     float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    std::vector<std::string> faces{"assets/cube/px.png", "assets/cube/nx.png", "assets/cube/py.png", "assets/cube/ny.png", "assets/cube/pz.png", "assets/cube/nz.png"};
    unsigned int cubemapTexture = loadCubemap(faces);

    // skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    
    //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    //create texture
    std::vector<std::string> dates = generateDates(2005);
    size_t date = 0;
    std::string globepath = "assets/low_res_globe/visible_globe_c1440_NR_BETA9-SNAP_" + dates[date] + "z.png";
    std::cout << globepath << std::endl;
    unsigned int earthtexture = loadTexture(globepath);    

    unsigned int nighttexture = loadTexture("assets/BlackMarble_2016_3km_gray.jpg");

    unsigned int moontexture = loadTexture("assets/moon2.png");


    glm::vec4 lightdir = glm::vec4(1.0, 1.0, 0.0, 0.0);

    //create matrix
    int SCR_WIDTH = 800;
    int SCR_HEIGHT = 600;
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //configure shader
    earthShader.use();
    earthShader.setInt("ourTexture", 0);
    earthShader.setMat4("model", model);
    earthShader.setMat4("view", view);
    earthShader.setMat4("projection", projection);

    glm::mat4 modelmoon = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4(1.0f);

    //configure shader
    moonShader.use();
    moonShader.setInt("ourTexture", 0);
    moonShader.setMat4("model", modelmoon);
    moonShader.setMat4("view", view);
    moonShader.setMat4("projection", projection);

    universeboxShader.use();
    universeboxShader.setInt("skybox", 0);
    glm::mat4 viewcube = glm::lookAt(glm::vec3(0,0,0), glm::vec3(0, 1, 0), glm::vec3(0,0,1));
    glm::mat4 projectioncube = glm::perspective(90.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    universeboxShader.setMat4("view", viewcube); //TODO
    universeboxShader.setMat4("projection", projectioncube);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    while (!glfwWindowShouldClose(window))
    {   
        glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        // draw earth
        earthShader.use();
        //glm::mat4 rot = glm::rotate(glm::mat4(1.0), glm::radians(15.0f*(float)date), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 rot = glm::rotate(glm::mat4(1.0), glm::radians((float)date), glm::vec3(0.0, 1.0, 0.0));
        earthShader.setVec4("lightdir", rot * lightdir);
        earthShader.setMat4("model", model);
        glBindVertexArray(earth.VERTEX_ARRAY_OBJECT);
        glActiveTexture(GL_TEXTURE0);
        earthShader.setInt("ourTexture", 0);
        glBindTexture(GL_TEXTURE_2D, earthtexture);
        //update texture
        date = (date + 1) % dates.size();
        std::cout << dates[date] << std::endl;
        globepath = "assets/low_res_globe/visible_globe_c1440_NR_BETA9-SNAP_" + dates[date] + "z.png";
        updateTexture(earthtexture, globepath);
        glActiveTexture(GL_TEXTURE0 + 1);
        earthShader.setInt("ourTexture2", 1);
        glBindTexture(GL_TEXTURE_2D, nighttexture);
        glDrawElements(GL_TRIANGLES, 3 * earth.indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
        
        // draw moon
        moonShader.use();
        moonShader.setVec4("lightdir", rot * lightdir);
        modelmoon = glm::mat4(1.0);
        modelmoon = glm::rotate(modelmoon, 0.5f * glm::radians((float)date), glm::vec3(0.0, 1.0, 0.0));
        rotation = modelmoon;
        modelmoon = glm::scale(modelmoon, glm::vec3(0.2, 0.2, 0.2));
        modelmoon = glm::translate(modelmoon, glm::vec3(8.0, 0.0, 0.0));
        moonShader.setMat4("model", modelmoon);
        moonShader.setMat4("rotation", rotation);
        glBindVertexArray(moon.VERTEX_ARRAY_OBJECT);
        glActiveTexture(GL_TEXTURE0);
        moonShader.setInt("ourTexture", 0);
        glBindTexture(GL_TEXTURE_2D, moontexture);
        glDrawElements(GL_TRIANGLES, 3 * moon.indices.size(), GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);

        // draw universe
        universeboxShader.use();
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
