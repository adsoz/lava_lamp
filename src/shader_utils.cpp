#include "shader_utils.h"
#include <fstream>
#include <iostream>
#include <vector>

GLuint compileShaderFromPath(const std::string &shaderPath, GLenum shaderType) {
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << shaderPath << std::endl;
        return 0;
    }
    std::string shaderSource = std::string((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

    const char *sourceCstr = shaderSource.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourceCstr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Failed to compile shaders (" << shaderPath << "): " << infoLog << std::endl;
        glDeleteShader(shader);
    }

    return shader;
}

GLuint makeProgram(const std::string &vsPath, const std::string &fsPath) {
    GLuint vs = compileShaderFromPath(vsPath, GL_VERTEX_SHADER);
    GLuint fs = compileShaderFromPath(fsPath, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Failed to link shader program: " << infoLog << std::endl;
        glDeleteProgram(program);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void saveProgramBinary(GLuint program, const std::string &filePath) {
    GLint length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
    if (length == 0) {
        std::cerr << "Failed to retrieve program binary length." << std::endl;
        return;
    }

    std::vector<GLubyte> binary(length);
    GLenum binaryFormat;
    glGetProgramBinary(program, length, nullptr, &binaryFormat, binary.data());

    // Save binary format and binary data to file
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return;
    }

    file.write(reinterpret_cast<const char *>(&binaryFormat), sizeof(binaryFormat));
    file.write(reinterpret_cast<const char *>(binary.data()), binary.size());
    file.close();

    std::cout << "Shader program binary saved to " << filePath << std::endl;
}

GLuint loadProgram(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filePath << std::endl;
        return 0;
    }

    // Read binary format and binary data
    GLenum binaryFormat;
    file.read(reinterpret_cast<char *>(&binaryFormat), sizeof(binaryFormat));

    file.seekg(0, std::ios::end);
    size_t binarySize = static_cast<size_t>(file.tellg()) - sizeof(binaryFormat);
    file.seekg(sizeof(binaryFormat), std::ios::beg);

    std::vector<GLubyte> binary(binarySize);
    file.read(reinterpret_cast<char *>(binary.data()), binarySize);
    file.close();

    // Create program and load binary
    GLuint program = glCreateProgram();
    glProgramBinary(program, binaryFormat, binary.data(), binarySize);

    // Verify program
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Failed to load program binary: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    std::cout << "Shader program loaded from binary file." << std::endl;
    return program;
}

bool binaryExists(const std::string &filePath) {
    std::ifstream file(filePath);
    return file.good();
}
