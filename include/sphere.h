#ifndef SPHERE_H_
#define SPHERE_H_
#include <glad/glad.h>
#include <vector>

class Sphere {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    Sphere(GLfloat radius, int sectors, int stacks);
};

#endif