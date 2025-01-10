#version 460 core

#define MAX_BLOBS 20

out vec4 FragColor;

layout(location = 0) uniform vec4 lightColor;
layout(location = 1) uniform vec3 lightPos;
layout(location = 3) uniform mat4 view;
layout(location = 2) uniform vec3 viewPos;
layout(location = 4) uniform mat4 projection;

layout(location = 5) uniform float width;
layout(location = 6) uniform float height;

layout(std140) uniform Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float subsurfaceRadius;
    float scatteringCoefficient;
    float transmissionCoefficent;
    float scatteringDistance;
};

layout(std140) uniform BlobData {
    vec4 blobs[MAX_BLOBS]; // x, y, z, radius
};

layout(location = 7) uniform int blobCount;
layout(location = 8) uniform float threshold;

layout(location = 9) uniform vec4 backgroundColor;
layout(location = 10) uniform float lightIntensity;

const float PI = 3.14159265359;

const float stepSize = 0.005;
const int maxSteps = 1000;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


float blobField(vec3 p) {
    float field = 0.0;
    for (int i = 0; i < blobCount; ++i) {
        field += blobs[i].w * blobs[i].w / distance(p, blobs[i].xyz);
    }
    return field;
}

vec3 estimateNormal(vec3 p) {
    float EPSILON = 0.0005;
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

    const float lightRadius = 0.05;
    float depth = 0.0;

    for (int i = 0; i < maxSteps; i++) {
        vec3 p = rayOrigin + depth * rayDir;

        float lightDist = distance(p, lightPos) - lightRadius;
        if (lightDist < stepSize) {
            FragColor = lightColor;
            return;
        }

        float field = blobField(p);

        if (field > threshold) { 

            vec3 N = normalize(estimateNormal(p));
            vec3 V = normalize(viewPos - p);

            vec3 Lo = vec3(0.0);

            // Light propagation through the blob
            vec3 L = normalize(p - lightPos);
            vec3 H = normalize(V + L);

            // Subsurface scattering attenuation
            float distInsideBlob = max(0.0, length(p - lightPos) - lightRadius);
            float ssAttenuation = exp(-scatteringCoefficient * distInsideBlob);  // Exponential decay

            float distance = length(lightPos - p);
            float attenuation = ssAttenuation / (distance * distance + 0.01);
            vec3 radiance = lightColor.xyz * attenuation * lightIntensity;

            vec3 F0 = vec3(0.04);
            F0 = mix(F0, albedo, metallic);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;
            
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;

            kD *= 1.0 - metallic;

        
            float NdotL = max(dot(N, L), 0.0);        
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;

            vec3 ambient = vec3(0.09) * albedo * ao;
            
            vec3 transmittedLight = mix(ambient, radiance, transmissionCoefficent) * attenuation;

            vec3 color = transmittedLight + Lo;  
            color = pow(color, vec3(1.0/2.2)); 
            FragColor = vec4(color, 1.0);
            
            // vec3 finalColor = (Lo + transmittedLight) * radiance;  // Without clamping here
            // finalColor = finalColor / (finalColor + vec3(1.0));  // Apply tone mapping or linear to sRGB conversion after this
            // FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
            return;
        }

        depth += stepSize;
    }

    FragColor = backgroundColor;
}