#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sphere.h"
#include "shader_utils.h"
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define BLOB_COUNT 2

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec4 objectColor(1.0f, 0.5f, 0.2f, 1.0f);
glm::vec4 lightColor(1.0f, 1.0f, 1.0f, 1.0f);

glm::vec3 positions[] = {
    glm::vec3(0.5f, 0.0f, 0.0f),
    glm::vec3(-0.5f, 0.0f, 0.0f),
};

float quadVertices[] = {
    // Positions (NDC)
    -1.0f, -1.0f,  // Bottom-left
     1.0f, -1.0f,  // Bottom-right
    -1.0f,  1.0f,  // Top-left

    -1.0f,  1.0f,  // Top-left
     1.0f, -1.0f,  // Bottom-right
     1.0f,  1.0f   // Top-right
};

// set the light source behind the camera
glm::vec3 lightPos(0.0f, 0.0f, 4.0f);
glm::vec3 viewPos(0.0f, 0.0f, 3.0f);

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *win) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        viewPos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        viewPos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        viewPos += cameraSpeed * glm::vec3(1.0f, 0.0f, 0.0);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        viewPos -= cameraSpeed * glm::vec3(1.0f, 0.0f, 0.0);
    if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        viewPos += cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0);
    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        viewPos -= cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0);
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

    // If the shader has already been compiled load it from binary, otherwise compile and save it
    GLuint blobProgram;
    std::string shaderBinaryPath = "build/shaders/blob_shader.bin";
    if (binaryExists(shaderBinaryPath)) {
        blobProgram = loadProgram(shaderBinaryPath);
    }
    else {
        blobProgram = makeProgram("shaders/blob_vs.glsl", "shaders/blob_fs.glsl");
        if (blobProgram) saveProgramBinary(blobProgram, shaderBinaryPath);
    }
    glUseProgram(blobProgram);

    struct Material {
        glm::vec4 color;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        float shininess;
        Material(
            glm::vec4 color, 
            glm::vec4 ambient, 
            glm::vec4 diffuse, 
            glm::vec4 specular, 
            float shininess
        ) : 
            color{color}, 
            ambient{ambient}, 
            diffuse{diffuse}, 
            specular{specular}, 
            shininess{shininess} 
        {};
    };

    Material objectMat(
        objectColor,
        glm::vec4(1.0f, 0.5f, 0.31f, 0.0f), 
        glm::vec4(1.0f, 0.5f, 0.31f, 0.0f), 
        glm::vec4(0.5f, 0.5f, 0.5f, 0.0f), 
        32.0
    );

    GLuint materialUBO;
    glGenBuffers(1, &materialUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, materialUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &objectMat, GL_STATIC_DRAW);
    
    GLuint materialBlockIndex = glGetUniformBlockIndex(blobProgram, "Material");
    glUniformBlockBinding(blobProgram, materialBlockIndex, 0);

    struct Blob {
        glm::vec4 pos_R;
        Blob(GLfloat x, GLfloat y, GLfloat z, GLfloat r) : pos_R{x, y, z, r} {};
        Blob(glm::vec3 pos, GLfloat r) : pos_R{pos.x, pos.y, pos.z, r} {};
    };

    // std::vector<Blob> blobs;
    // for (int i = 0; i < BLOB_COUNT; i++) {
    //     blobs.emplace_back(positions[i], 1.0f);
    // }

    Blob blobs[2] = { Blob(positions[0], 0.5f), Blob(positions[1], 0.5f) }; // for now

    GLuint blobsUBO;
    glGenBuffers(1, &blobsUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, blobsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(blobs), blobs, GL_STATIC_DRAW);

    GLuint blobsBlockIndex = glGetUniformBlockIndex(blobProgram, "BlobData");
    glUniformBlockBinding(blobProgram, blobsBlockIndex, 1);

    glUniform4fv(0, 1, glm::value_ptr(lightColor));
    glUniform3fv(1, 1, glm::value_ptr(lightPos));


    glUniform3fv(2, 1, glm::value_ptr(viewPos));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT), 0.1f, 100.0f);
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1f(5, static_cast<float>(WINDOW_WIDTH));
    glUniform1f(6, static_cast<float>(WINDOW_HEIGHT));
    
    glUniform1f(7, 1.0f); // threshold for ray marching

    GLuint quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind the VAO
    glBindVertexArray(0);

    


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

        glUniform3fv(2, 1, glm::value_ptr(viewPos));
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, viewPos);
        // view = glm::lookAt(viewPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(view));
        
        // move the blob positions along the x axis
        for (int i = 0; i < BLOB_COUNT; i++) {
            blobs[i].pos_R.x += (i % 2 == 0 ? 1.0f : -1.0f) * 0.002f * sinf(currentFrame);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, blobsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(blobs), blobs);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}