#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 tangent;

layout(location = 0) out vec4 outColor;

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

layout(set = 0, binding = 1) uniform sampler2D baseColorMap;
layout(set = 0, binding = 2) uniform sampler2D metallicRoughnessMap;

layout(push_constant) uniform Material {
    vec4 baseColorFactor;
    float metallic;
    float roughness;
} mat;

const float PI = 3.14159265359;

// ===== PBR FUNCTIONS =====
float DistributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float g1 = NdotV / (NdotV * (1.0 - k) + k);
    float g2 = NdotL / (NdotL * (1.0 - k) + k);
    return g1 * g2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ===== MAIN =====
void main() {
    vec3 N = normalize(normal);
    vec3 V = normalize(ubo.camPos.xyz - fragPos);

    vec3 baseColor = texture(baseColorMap, uv).rgb * mat.baseColorFactor.rgb;

    float metallic = mat.metallic;
    float roughness = mat.roughness;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < 4; i++) {
        vec3 L = normalize(ubo.lightPos[i].xyz - fragPos);
        vec3 H = normalize(V + L);

        float dist = length(ubo.lightPos[i].xyz - fragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = ubo.lightColor[i].rgb * attenuation;

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float D = DistributionGGX(NdotH, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        vec3 F = FresnelSchlick(HdotV, F0);

        vec3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        Lo += (kD * baseColor / PI + spec) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * baseColor;
    vec3 color = ambient + Lo;

    // Tone mapping
    color = color / (color + vec3(1.0));

    // Gamma
    color = pow(color, vec3(1.0 / ubo.gamma));

    outColor = vec4(color, 1.0);
}