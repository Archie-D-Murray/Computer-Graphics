#include "mesh.hpp"

Mesh::Mesh(vector<Vertex> vertices,
     vector<unsigned int> indices,
     vector <Texture> textures)
{
    this->vertices = vertices;
    this->indices  = indices;
    this->textures = textures;
    
//    setupMesh();
}
