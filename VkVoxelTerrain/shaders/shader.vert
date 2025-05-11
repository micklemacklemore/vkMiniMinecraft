#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 viewproj;
} ubo;

layout(push_constant) uniform PushConstants {
    vec4 offset; 
    vec4 color; 
    vec4 padding[2]; 
} pc;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Apply offset as a translation to the vertex position
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    worldPos.xyz += pc.offset.xyz;

    gl_Position = ubo.viewproj * worldPos;
    fragColor = pc.color.rgb;
    fragTexCoord = inTexCoord;
}