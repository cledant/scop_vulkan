#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ModelUBO {
    mat4 view_proj;
} ubo;

layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;
layout(location = 5) in mat4 instanceMatrix;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.view_proj * instanceMatrix * vec4(inVertexPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}