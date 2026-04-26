#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;
layout(location = 3) out vec4 tangent;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 lightPos[4];
    vec4 lightColor[4];
    vec4 camPos;
    float exposure;
    float gamma;
} ubo;

void main() {
    vec4 worldPos = ubo.model * vec4(inPos, 1.0);
    fragPos = worldPos.xyz;
    normal = mat3(ubo.model) * inNormal;
    uv = inUV;
    tangent = inTangent;

    gl_Position = ubo.proj * ubo.view * worldPos;
}