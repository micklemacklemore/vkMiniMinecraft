#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;  // Interpolated normal from vertex shader

layout(location = 0) out vec4 outColor;

void main() {
    // Define a directional light (world space)
    vec3 lightDir = normalize(vec3(1.0, 1.0, -1.0));  // light coming from top-right front

    // Normalize interpolated normal
    vec3 N = normalize(fragNormal);

    // Lambert diffuse factor
    float diff = max(dot(N, lightDir), 0.4);

    // Sample the texture and apply diffuse shading and vertex color
    vec4 texColor = texture(texSampler, fragTexCoord);
    vec3 shadedColor = texColor.rgb * diff;

    outColor = vec4(shadedColor, texColor.a);
}
