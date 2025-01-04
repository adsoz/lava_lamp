#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout(std140, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 fragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal; // avoid calculating inverse here and pass normal matrix instead
}
