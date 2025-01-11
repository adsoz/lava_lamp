#version 460 core

#define MAX_BLOBS 20

out vec4 FragColor;

layout(location = 0) uniform vec4 lightColor;
layout(location = 1) uniform vec3 lightPos;

layout(location = 2) uniform vec3 viewPos;
layout(location = 3) uniform mat4 view;
layout(location = 4) uniform mat4 projection;

layout(location = 5) uniform float width;
layout(location = 6) uniform float height;

layout(std140) uniform Material {
    vec4 color;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

layout(std140) uniform BlobData {
    vec4 blobs[MAX_BLOBS]; // x, y, z, radius
};
layout(location = 7) uniform int blobCount;
layout(location = 8) uniform float threshold;

layout(location = 9) uniform vec4 backgroundColor;

const float stepSize = 0.01;
const int maxSteps = 1000;

float blobField(vec3 p) {
    float field = 0.0;
    for (int i = 0; i < blobCount; ++i) {
        field += blobs[i].w * blobs[i].w / distance(p, blobs[i].xyz);
    }
    return field;
}

vec3 estimateNormal(vec3 p) {
    float EPSILON = 0.001;
    return normalize(vec3(
        blobField(p + vec3(EPSILON, 0, 0)) - blobField(p - vec3(EPSILON, 0, 0)),
        blobField(p + vec3(0, EPSILON, 0)) - blobField(p - vec3(0, EPSILON, 0)),
        blobField(p + vec3(0, 0, EPSILON)) - blobField(p - vec3(0,  0, EPSILON))
    ));
}

void main() {
    vec2 ndc = gl_FragCoord.xy / vec2(width, height) * 2.0 - 1.0; // convert screen space to normalized device coords
    ndc.y = -ndc.y; // flip y axis for OpenGL coord system

    vec4 rayStartNDC = vec4(ndc, -1.0, 1.0); // near plane
    vec4 rayEndNDC = vec4(ndc, 1.0, 1.0); // far plane

    // transform ndc to world space
    mat4 invProjView = inverse(projection * view);
    vec4 rayStartWorld = invProjView * rayStartNDC;
    vec4 rayEndWorld = invProjView * rayEndNDC;

    // perspective divide
    rayStartWorld /= rayStartWorld.w;
    rayEndWorld /= rayEndWorld.w;

    vec3 rayDir = normalize(rayEndWorld.xyz - rayStartWorld.xyz);
    vec3 rayOrigin = viewPos;

    float depth = 0.0;
    for (int i = 0; i < maxSteps; i++) {
        vec3 p = rayOrigin + depth * rayDir;
        float field = blobField(p);

        if (field > threshold) { 

            vec4 ambientColor = ambient * lightColor;

            vec3 norm = estimateNormal(p);

            vec3 lightDir = normalize(lightPos - p);
            float diff = max(dot(norm, lightDir), 0.0);
            vec4 diffuseColor = diffuse * diff * lightColor;

            vec3 viewDir = normalize(viewPos - p);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec4 specularColor = specular * spec * lightColor;

            FragColor = color * (diffuseColor + ambientColor + specularColor);
            return;
        }


        depth += stepSize;
    }

    FragColor = backgroundColor;

}
