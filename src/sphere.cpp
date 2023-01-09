#include <stdio.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>

#define X .525731112119133606f
#define Z .850650808352039932f



class Sphere {
    public:
        std::vector<glm::vec3> vertices = {
            glm::vec3(-X, 0.0f, Z), glm::vec3(X, 0.0f, Z), glm::vec3(-X, 0.0f, -Z), glm::vec3(X, 0.0f, -Z),    
            glm::vec3(0.0f, Z, X), glm::vec3(0.0f, Z, -X), glm::vec3(0.0f, -Z, X), glm::vec3(0.0f, -Z, -X),    
            glm::vec3(Z, X, 0.0f), glm::vec3(-Z, X, 0.0f), glm::vec3(Z, -X, 0.0f), glm::vec3(-Z, -X, 0.0f)
        };

        std::vector<glm::uvec3> indices = {
            glm::uvec3(0,4,1), glm::uvec3(0,9,4), glm::uvec3(9,5,4), glm::uvec3(4,5,8), glm::uvec3(4,8,1),    
            glm::uvec3(8,10,1), glm::uvec3(8,3,10), glm::uvec3(5,3,8), glm::uvec3(5,2,3), glm::uvec3(2,7,3),    
            glm::uvec3(7,10,3), glm::uvec3(7,6,10), glm::uvec3(7,11,6), glm::uvec3(11,0,6), glm::uvec3(0,1,6), 
            glm::uvec3(6,1,10), glm::uvec3(9,0,11), glm::uvec3(9,11,2), glm::uvec3(9,2,5), glm::uvec3(7,2,11)
        };
    Sphere(int degree) {
        for (size_t i = 0; i < degree; i++) smoothen();
    }

    void smoothen(){
        size_t orignalvertexlength = vertices.size();
        std::vector<glm::uvec2> edges;
        std::vector<glm::uvec3> triangles2edges, newindices;
        triangles2edges = std::vector<glm::uvec3>(indices.size());
        for(size_t i = 0; i < indices.size(); i++) {
            auto edge = indices[i].x >= indices[i].y ? glm::uvec2(indices[i].x, indices[i].y) : glm::uvec2(indices[i].y, indices[i].x);
            auto found = std::find(edges.begin(), edges.end(), edge);
            auto idx = found - edges.begin();
            if(found == edges.end()) edges.push_back(edge);
            triangles2edges[i][0] = idx;

            edge = indices[i].x >= indices[i].z ? glm::uvec2(indices[i].x, indices[i].z) : glm::uvec2(indices[i].z, indices[i].x);
            found = std::find(edges.begin(), edges.end(), edge);
            idx = found - edges.begin();
            if(found == edges.end()) edges.push_back(edge);
            triangles2edges[i][1] = idx;

            edge = indices[i].y >= indices[i].z ? glm::uvec2(indices[i].y, indices[i].z) : glm::uvec2(indices[i].z, indices[i].y);
            found = std::find(edges.begin(), edges.end(), edge);
            idx = found - edges.begin();
            if(found == edges.end()) edges.push_back(edge);
            triangles2edges[i][2] = idx;
        }
        for (auto edge : edges) {
            vertices.push_back(glm::normalize(vertices.at(edge.x) + vertices.at(edge.y)));
        }
        for (size_t i = 0; i < triangles2edges.size(); i++) {
            size_t v1 = indices[i].x;
            size_t v2 = indices[i].y;
            size_t v3 = indices[i].z;
            size_t v4 = orignalvertexlength + triangles2edges[i].x;
            size_t v5 = orignalvertexlength + triangles2edges[i].y;
            size_t v6 = orignalvertexlength + triangles2edges[i].z;
            newindices.push_back(glm::uvec3(v1, v4, v5));
            newindices.push_back(glm::uvec3(v4, v2, v6));
            newindices.push_back(glm::uvec3(v4, v6, v5));
            newindices.push_back(glm::uvec3(v5, v6, v3));
        }
        indices = newindices;
    }
};


/*vector kanten
vector 3usize (index in kanten)

1. Dreiecke -> Linie
2. Unterteile Linie
3. Unterteile Dreiecke
*/