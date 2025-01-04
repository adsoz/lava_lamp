#ifndef SPHERE_H_
#define SPHERE_H_
#include <glad/glad.h>
#include <vector>

class Sphere {
    GLuint VBO;
    GLuint VAO;
    std::vector<GLfloat> vertices;
    // generates vertices (including interleaved normals)
    void generateVertices(GLfloat radius, int sectors, int stacks);
    // generates indices for drawing based on the vertices
    void generateIndices();
  public:
    Sphere(GLfloat radius, int sectors, int stacks);
    void draw();

};

#endif