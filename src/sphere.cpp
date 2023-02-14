#include <stdio.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
#include <map>

#define X .525731112119133606f
#define Z .850650808352039932f
#define PI 3.14159265358979323846f


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

    std::vector<glm::vec2> getTextureCoor () {
        // standard texture mapping via trigonometric functions
        std::vector<glm::vec2> texturecoor;
        for (auto v : vertices) texturecoor.push_back(glm::vec2(-atan2(v.x, v.z)/PI * 0.5 + 0.5f, acos(v.y)/PI));        
        // identifying the seam
        std::vector<size_t> seam;
        for (size_t i = 0; i < indices.size(); i++) {
            if ((texturecoor[indices[i].x].x < 0.3f || texturecoor[indices[i].y].x < 0.3f || texturecoor[indices[i].z].x < 0.3f) 
                &&  (texturecoor[indices[i].x].x > 0.7f || texturecoor[indices[i].y].x > 0.7f || texturecoor[indices[i].z].x > 0.7f)) {
                seam.push_back(i);
            }
        }
        // fixing the seam
        size_t i = 0;
        std::map<size_t, size_t> mapping;
        while (i < seam.size()) {
            auto vertexid = indices[seam[i]].x;
            if(texturecoor[vertexid].x < 0.3f) {
                auto search = mapping.find(vertexid);
                if (search != mapping.end()) {
                    indices[seam[i]].x = search->second;
                } else {
                    vertices.push_back(vertices[vertexid]);
                    texturecoor.push_back(glm::vec2(texturecoor[vertexid].x + 1.0f, texturecoor[vertexid].y));
                    indices[seam[i]].x = vertices.size() - 1;
                }
            }
            vertexid = indices[seam[i]].y;
            if(texturecoor[vertexid].x < 0.3f) {
                auto search = mapping.find(vertexid);
                if (search != mapping.end()) {
                    indices[seam[i]].y = search->second;
                } else {
                    vertices.push_back(vertices[vertexid]);
                    texturecoor.push_back(glm::vec2(texturecoor[vertexid].x + 1.0f, texturecoor[vertexid].y));
                    indices[seam[i]].y = vertices.size() - 1;
                }
            }
            vertexid = indices[seam[i]].z;
            if(texturecoor[vertexid].x < 0.3f) {
                auto search = mapping.find(vertexid);
                if (search != mapping.end()) {
                    indices[seam[i]].z = search->second;
                } else {
                    vertices.push_back(vertices[vertexid]);
                    texturecoor.push_back(glm::vec2(texturecoor[vertexid].x + 1.0f, texturecoor[vertexid].y));
                    indices[seam[i]].z = vertices.size() - 1;
                }
            }
            i++;
        }
        return texturecoor;
    }

    void smoothen(){
        size_t orignalvertexlength = vertices.size();
        std::vector<glm::uvec2> edges;
        std::vector<glm::uvec3> triangles2edges, newindices;
        triangles2edges = std::vector<glm::uvec3>(indices.size());
        // create a mapping triag -> edges
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
        // create a new vertex on each edge
        for (auto edge : edges) {
            vertices.push_back(glm::normalize(vertices.at(edge.x) + vertices.at(edge.y)));
        }
        // split every triag in 4
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