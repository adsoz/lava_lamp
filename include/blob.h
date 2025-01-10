#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Blob {
    static int n;
    static const float speed;
    int index{0};
    glm::vec3 pos{0.0f};
    GLfloat radius{0.0f};
    glm::vec3 vel{0.0f};
    float yRange;

    Blob(glm::vec3 pos, glm::vec3 vel, GLfloat r, GLfloat yRange) 
    : pos{pos}, vel{vel}, radius{r}, index{n++}, yRange{yRange} {};

    void update(float deltaTime);
    
};