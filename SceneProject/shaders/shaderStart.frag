#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

// Lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// Texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Fog
uniform float fogDensity;

// Night Mode
uniform bool isNightMode;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

// Point light
uniform int pointinit;
uniform vec3 lightPointPos;

float constant = 1.0f;
float linear = 0.09f;
float quadratic = 0.1;

float ambientPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 32.0f;

uniform mat4 view;

vec3 computeLightComponents() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 halfVector = normalize(lightDirN + viewDirN);

    ambient = ambientStrength * lightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;

    if (isNightMode) {
        ambient *= 0.1;
        diffuse *= 0.2;
        specular *= 0.3;
    }
    
    return (ambient + diffuse + specular);
}

float computeFog() {
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

float computeShadow() {
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5f + 0.5f; // [0,1]
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = 0.0005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    if (normalizedCoords.z > 1.0f) return 0.0f;
    return shadow;
}

vec3 computePointLight(vec4 lightPosEye) {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 ambient = ambientPoint * lightColor;
    vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    vec3 halfVector = normalize(lightDirN + viewDirN);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininessPoint);
    vec3 specular = specularStrengthPoint * specCoeff * lightColor;
    float distance = length(lightPosEye.xyz - fPosEye.xyz);
    float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
    return (ambient + diffuse + specular) * att;
}

void main() {
    vec3 light = computeLightComponents();

    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;

    ambient *= baseColor;
    diffuse *= baseColor;
    specular *= texture(specularTexture, fTexCoords).rgb;

    float shadow = computeShadow();

    // Point light calculations
    if (pointinit == 1) {
        vec4 lightPointPosEye = view * vec4(lightPointPos, 1.0f);
        light += computePointLight(lightPointPosEye);
    }

    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.98f, 0.851f, 0.667f, 1.0f); // Default fog color

    if (isNightMode) {
        fogColor = vec4(0.05, 0.05, 0.1, 1.0); // Darker fog for night
    }

    vec4 colorWithShadow = vec4(color, 1.0f);
    fColor = mix(fogColor, min(colorWithShadow * vec4(light, 1.0f), 1.0f), fogFactor);
}
