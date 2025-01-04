#include <vector>
#include "sphere.h"
#include <cmath>

const double PI = 3.14159265358979323846;

// Initializes the sphere vertices and normals
Sphere::Sphere(GLfloat radius, int sectors, int stacks) {

    generateVertices(radius, sectors, stacks);

    generateIndices();    

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), vertices, GL_STATIC_DRAW);

    // vertices 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);
}

void Sphere::generateVertices(GLfloat radius, int sectors, int stacks) {
    // divide the whole circle (2PI) to get the sector size
    float sectorSize = 2 * PI / sectors;
    // divide the vertical half of the circle (PI) by stacks
    float stackSize = PI / stacks;
    // theta and phi are the angle from the meridian and equator
    float theta, phi;
    for (int i = 0; i <= stacks; i++) {
        // then to get phi we subtract the half circle by amount we've stepped
        float phi = PI / 2 - i * stackSize;
        float rcosPhi = radius * cosf(phi);
        float rsinPhi = radius * sinf(phi);

        for (int j = 0; j <= sectors; j++) {
            GLfloat theta = j * sectorSize;

            float x = rcosPhi * cosf(theta);
            float y = rcosPhi * sinf(theta);
            float z = rsinPhi;

            // vertices
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normals
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
        }
    }
}

void Sphere::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

}