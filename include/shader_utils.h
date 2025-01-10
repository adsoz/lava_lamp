#include <glad/glad.h>
#include <string>

// compiles shader from a shaderPath, returns 0 if error occurs
GLuint compileShaderFromPath(const std::string &shaderPath, GLenum shaderType);

// generates a shader program from vertex shader path and fragment shader path, returns 0 if error occurs
GLuint makeProgram(const std::string &vsPath, const std::string &fsPath);

// saves program to path 
void saveProgramBinary(GLuint program, const std::string &filePath);

// loads program from path
GLuint loadProgram(const std::string &filePath);

// checks if a shader binary exists at given path
bool binaryExists(const std::string &filePath);