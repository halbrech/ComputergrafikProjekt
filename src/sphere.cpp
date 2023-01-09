#include <stdio.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#define X .525731112119133606f
#define Z .850650808352039932f



class Sphere {
    public:
        std::vector<glm::vec3> vertices = {
            glm::vec3(-X, 0.0f, Z), glm::vec3(X, 0.0f, Z), glm::vec3(-X, 0.0f, -Z), glm::vec3(X, 0.0f, -Z),    
            glm::vec3(0.0f, Z, X), glm::vec3(0.0f, Z, -X), glm::vec3(0.0f, -Z, X), glm::vec3(0.0f, -Z, -X),    
            glm::vec3(Z, X, 0.0f), glm::vec3(-Z, X, 0.0f), glm::vec3(Z, -X, 0.0f), glm::vec3(-Z, -X, 0.0f)
        };

        std::vector<glm::vec3> indices = {
            glm::vec3(0,4,1), glm::vec3(0,9,4), glm::vec3(9,5,4), glm::vec3(4,5,8), glm::vec3(4,8,1),    
            glm::vec3(8,10,1), glm::vec3(8,3,10), glm::vec3(5,3,8), glm::vec3(5,2,3), glm::vec3(2,7,3),    
            glm::vec3(7,10,3), glm::vec3(7,6,10), glm::vec3(7,11,6), glm::vec3(11,0,6), glm::vec3(0,1,6), 
            glm::vec3(6,1,10), glm::vec3(9,0,11), glm::vec3(9,11,2), glm::vec3(9,2,5), glm::vec3(7,2,11)
        };
    Sphere() {
        //triangles.reserve(20);
        //for(indice i : indices) glm::vec3(
        //    triangles.push_back(vertices[i.v1]);
        //    triangles.push_back(vertices[i.v2]);
        //    triangles.push_back(vertices[i.v3]);
        //}
    }
};