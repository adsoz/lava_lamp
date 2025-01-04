#include <vector>
#include "sphere.h"
#include <cmath>

const double PI = 3.14159265358979323846;



Sphere::Sphere(GLfloat radius, int sectors, int stacks) {
    float sectorStep = 2 * PI / sectors;
    float stackStep = PI / stacks;
    float theta, phi;
    for (int i = 0; i <= stacks; i++) {
        GLfloat phi = PI / 2 - i * stackStep;

        for (int j = 0; j <= sectors; j++) {
            GLfloat theta = j * 
        }
    }
}