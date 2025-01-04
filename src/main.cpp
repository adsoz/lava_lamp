#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 objectColor(1.0f, 0.5f, 0.2f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 positions[] = {
    glm::vec3(0.5f, 0.0f, 0.0f),
    glm::vec3(-0.5f, 0.0f, 0.0f),
};

// set the light source behind the camera
glm::vec3 lightPosition(0.0f, 0.0f, 4.0f);

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *win) {
    float cubeSpeed = 2.5f * deltaTime;

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }

    // if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    //     cubePosition += cubeSpeed * glm::vec3(0.0f, 1.0f, 0.0);
    // if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    //     cubePosition -= cubeSpeed * glm::vec3(0.0f, 1.0f, 0.0);
    // if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    //     cubePosition += cubeSpeed * glm::vec3(1.0f, 0.0f, 0.0);
    // if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    //     cubePosition -= cubeSpeed * glm::vec3(1.0f, 0.0f, 0.0);
    // if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    //     cubePosition += cubeSpeed * glm::vec3(0.0f, 0.0f, 1.0);
    // if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    //     cubePosition -= cubeSpeed * glm::vec3(0.0f, 0.0f, 1.0);
}

const GLuint loadShaderBinary(const unsigned char *shaderBinary, const size_t binarySize, const GLenum shaderType) {
    if (!shaderBinary || !binarySize) {
        std::cerr << "Invalid shader binary data." << std::endl;
        return 0;
    }
    const char *entryPoint = "main";
    const GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }

    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, shaderBinary, binarySize);

    glSpecializeShader(shader, entryPoint, 0, nullptr, nullptr);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Failed to specialize shader" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// probably refactor this to take more than just fs and vs
const GLuint createShaderProgramFromBinaries(const unsigned char *vs_binary, const size_t vs_size, const unsigned char *fs_binary, const size_t fs_size) {
    GLuint program = glCreateProgram();
    const GLuint vertexShader = loadShaderBinary(vs_binary, vs_size, GL_VERTEX_SHADER);
    const GLuint fragmentShader = loadShaderBinary(fs_binary, fs_size, GL_FRAGMENT_SHADER);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Failed to link program" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lava Lamp", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // load the precompiled shader binaries and attach them to the objectProgram
    GLuint objectProgram = createShaderProgramFromBinaries(object_vs_binary, object_vs_binary_size, object_fs_binary, object_fs_binary_size);
    // GLuint lightSourceProgram = createShaderProgramFromBinaries(lightSource_vs_binary, lightSource_vs_binary_size, lightSource_fs_binary, lightSource_fs_binary_size);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    GLuint cubeVAO;
    glGenVertexArrays(1, &cubeVAO);

    glBindVertexArray(cubeVAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);
    

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);

    // render loop
    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // std::cout << "FPS: " << 1 / deltaTime << '\n';

        // input
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the cubes
        glUseProgram(objectProgram);
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(3, 1, glm::value_ptr(objectColor));
        glUniform3fv(4, 1, glm::value_ptr(lightColor));
        glUniform3fv(5, 1, glm::value_ptr(lightPosition));
        for (int i = 0; i < 2; i++) {
            glm::mat4 model(1.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, glm::vec3(0.4f));
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
            // render the cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(GLfloat));
        }

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}