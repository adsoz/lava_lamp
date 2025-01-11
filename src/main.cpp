#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>
#include <random>

#include "shader_utils.h"
#include "pnoise.h"
#include "blob.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 1000
// #define WINDOW_WIDTH 1050
// #define WINDOW_HEIGHT 608


float deltaTime = 0.0f;
float lastFrame = 0.0f;

float quadVertices[] = {
    -1.0f, -1.0f,  // Bottom-left
     1.0f, -1.0f,  // Bottom-right
    -1.0f,  1.0f,  // Top-left

    -1.0f,  1.0f,  // Top-left
     1.0f, -1.0f,  // Bottom-right
     1.0f,  1.0f   // Top-right
};

// set the light source behind the camera
glm::vec3 lightPos(0.0f, -1.0f, -3.0f);
glm::vec4 lightColor(1.0f, 1.0f, 1.0f, 1.0f);
float lightIntensity = 1.0f;

glm::vec3 viewPos(0.0f, 0.0f, 0.0f);


float halfFrustumHeight = 3.0f * tanf(glm::radians(45.0f) / 2.0f);
float halfFrustumWidth = halfFrustumHeight * (static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT));


void framebuffer_size_callback(GLFWwindow *win, int width, int height);
void processInput(GLFWwindow *win);


#ifdef USE_PHONG
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
#else
struct Material {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float subsurfaceRadius;
    float scatteringCoefficient;
    float transmissionCoefficent;
    float scatteringDistance;
    Material (
        glm::vec3 albedo,
        float metallic,
        float roughness,
        float ao,
        float subsurfaceRadius,
        float scatteringCoefficient,
        float transmissionCoefficent,
        float scatteringDistance
    ) :
        albedo{albedo},
        metallic{metallic},
        roughness{roughness},
        ao{ao},
        subsurfaceRadius{subsurfaceRadius},
        scatteringCoefficient{scatteringCoefficient},
        transmissionCoefficent{transmissionCoefficent},
        scatteringDistance{scatteringDistance}
    {};
};
#endif

int main() {
    PerlinNoise pn;

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

    // initialize imGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // If the shader has already been compiled load it from binary, otherwise compile and save it
    GLuint blobProgram;
    std::string shaderBinaryPath = "build/shaders/blob_shader.bin";
    if (binaryExists(shaderBinaryPath)) {
        blobProgram = loadProgram(shaderBinaryPath);
    }
    else {
#ifdef USE_PHONG
        blobProgram = makeProgram("shaders/blob_vs.glsl", "shaders/blob_fs.glsl");
#else
        blobProgram = makeProgram("shaders/blob_vs.glsl", "shaders/blob_pbr_fs.glsl");
#endif
        if (blobProgram) saveProgramBinary(blobProgram, shaderBinaryPath);
    }
    glUseProgram(blobProgram);

#ifdef USE_PHONG
    // Create and bind blob material to shader
    Material objectMat(
        glm::vec4(1.0f, 0.6f, 0.0f, 1.0f),
        glm::vec4(1.0f, 0.5f, 0.31f, 0.0f), 
        glm::vec4(1.0f, 0.5f, 0.31f, 0.0f), 
        glm::vec4(0.2f, 0.2f, 0.2f, 0.0f), 
        32.0f
    );
#else
    Material objectMat(
        glm::vec3(1.0f, 0.6f, 0.0f), // albedo
        0.1, // metallic,
        0.1, // roughness,
        1.0, // ao,
        0.2, // subsurfaceRadius,
        0.1, // scatteringCoefficient,
        0.0, // transmissionCoefficent,
        1.0 // scatteringDistance
    );
#endif

    GLuint materialUBO;
    glGenBuffers(1, &materialUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, materialUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &objectMat, GL_STATIC_DRAW);
    
    GLuint materialBlockIndex = glGetUniformBlockIndex(blobProgram, "Material");
    glUniformBlockBinding(blobProgram, materialBlockIndex, 0);

    
    // Init blobs
    int blobCount = 14;
    float speed = 0.25f;
    std::vector<Blob> blobs;
    for (int i = 0; i < blobCount; i++) {
        glm::vec3 pos{
            glm::clamp(pn.perlinNoise(glfwGetTime() + i + 1000.0f, 2, 1.0f) * halfFrustumWidth, -halfFrustumWidth, halfFrustumWidth), 
            glm::clamp(pn.perlinNoise(glfwGetTime() + i + 1000.0f, 2, 1.0f) * halfFrustumHeight, -halfFrustumHeight, halfFrustumHeight), 
            -3.0f
        };
        glm::vec3 vel{
            pn.perlinNoise(glfwGetTime() + pos.x + i, 6, 1.0f) * speed, 
            pn.perlinNoise(glfwGetTime() + pos.y + i, 6, 1.0f) * speed, 
            0.0f
        };
        blobs.emplace_back(pos, vel, 0.2f, halfFrustumHeight);
    }

    std::vector<glm::vec4> blobData(blobCount);
    for (int i = 0; i < blobCount; i++) {
        blobData.emplace_back(blobs[i].pos, blobs[i].radius);
    }

    // Bind other uniforms
    GLuint blobsUBO;
    glGenBuffers(1, &blobsUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, blobsUBO);
    glBufferData(GL_UNIFORM_BUFFER, blobData.size() * sizeof(glm::vec4), blobData.data(), GL_DYNAMIC_DRAW);

    GLuint blobsBlockIndex = glGetUniformBlockIndex(blobProgram, "BlobData");
    glUniformBlockBinding(blobProgram, blobsBlockIndex, 1);

    glUniform4fv(0, 1, glm::value_ptr(lightColor));
    glUniform3fv(1, 1, glm::value_ptr(lightPos));


    glUniform3fv(2, 1, glm::value_ptr(viewPos));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT), 0.1f, 100.0f);
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1f(5, static_cast<float>(WINDOW_WIDTH));
    glUniform1f(6, static_cast<float>(WINDOW_HEIGHT));
    
    glUniform1i(7, blobCount);
    glUniform1f(8, 1.0f); // threshold for ray marching
    glUniform4fv(9, 1, glm::value_ptr(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
#ifndef USE_PHONG
    glUniform1f(10, lightIntensity); 
#endif
    // Bind screen quads
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
        // input
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Blob Properties");
        ImGui::SliderFloat("Blob Speed", &Blob::speed, 0.0f, 10.0f);
        ImGui::Text("Modify Blob Material");

#ifdef USE_PHONG
        static glm::vec3 color = objectMat.color;
        if (ImGui::ColorEdit3("Blob Color", glm::value_ptr(color))) {
            objectMat.color = glm::vec4(color, 1.0f);
            glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &objectMat);
        }
        ImGui::Separator();
        ImGui::SliderFloat4("Ambient Strength", &objectMat.ambient[0], 0.0f, 1.0f);
        ImGui::SliderFloat4("Diffuse Strength", &objectMat.diffuse[0], 0.0f, 1.0f);
        ImGui::SliderFloat4("Specular Strength", &objectMat.specular[0], 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &objectMat.shininess, 0.0f, 1.0f);
        
#else
        static glm::vec3 albedo = objectMat.albedo;
        if (ImGui::ColorEdit3("Blob Color", glm::value_ptr(albedo))) {
            objectMat.albedo = albedo;
            glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &objectMat);
        }
        ImGui::Separator();

        ImGui::SliderFloat("Metallic", &objectMat.metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &objectMat.roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Occlusion", &objectMat.ao, 0.0f, 1.0f);
        ImGui::SliderFloat("subsurfaceRadius", &objectMat.subsurfaceRadius, 0.0f, 10.0f);
        ImGui::SliderFloat("scatteringCoefficent", &objectMat.scatteringCoefficient, 0.0f, 1.0f);
        ImGui::SliderFloat("transmissionCoefficent", &objectMat.transmissionCoefficent, 0.0f, 1.0f);
        ImGui::SliderFloat("scatteringDistance", &objectMat.scatteringDistance, 0.0f, 10.0f);
#endif        

        // Update the uniform buffer with the new values
        glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &objectMat);


        ImGui::End();

#ifndef USE_PHONG
        ImGui::Begin("Light Properties");
        ImGui::Text("Modify Light Properties");
        if (ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor))) {
            glUniform4fv(0, 1, glm::value_ptr(lightColor));
        }
        ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 10.0f);
        glUniform1f(10, lightIntensity); // light intensity

        ImGui::End();
#endif
        ImDrawList *drawList = ImGui::GetForegroundDrawList();

        ImVec2 pos(10.0f, 10.0f);
        ImU32 textColor = IM_COL32(255, 255, 0, 255);
        char fpsText[25];
        std::snprintf(fpsText, sizeof(fpsText), "FPS: %.2f", 1.0f / deltaTime);
        drawList->AddText(pos, textColor, fpsText);



        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform3fv(2, 1, glm::value_ptr(viewPos));
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(viewPos, glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(view));

        // update blob positions and blobData for the uniform block
        for (int i = 0; i < blobCount; i++) {
            blobs[i].update(deltaTime);
            blobData[i] = glm::vec4(blobs[i].pos, blobs[i].radius);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, blobsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, blobData.size() * sizeof(glm::vec4), blobData.data());

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *win) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, true);
    }
}
