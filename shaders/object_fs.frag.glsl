#version 460 core
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 fragPos;

layout(location = 0) out vec4 FragColor;

layout(std140, binding = 1) uniform Light {
    vec3 objectColor;
    vec3 lightColor;
    vec3 lightPos;
}

layout(std140, binding = 2) uniform Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// layout(location = 6) uniform Material material;
// Material material = {vec3(1.0, 0.5, 0.31), vec3(1.0, 0.5, 0.31), vec3(0.5, 0.5, 0.5), 32.0};

void main()
{
    vec3 ambient = material.ambient * lightColor;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = material.diffuse * diff * lightColor;

    vec3 viewPos = vec3(0.0, 0.0, -3.0);
    float specularStrength = 0.1;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = material.specular * spec * lightColor;


    FragColor = vec4(objectColor * (diffuse + ambient + specular), 1.0);
}
