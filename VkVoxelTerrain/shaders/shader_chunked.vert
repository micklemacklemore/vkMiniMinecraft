#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 viewproj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal; 
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal; 

void main() {
    gl_Position = ubo.viewproj * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor; 
    fragTexCoord = inTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    outNormal = normalize(normalMatrix * inNormal);
}