#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view_proj;
} ubo;

layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.view_proj * vec4(inVertexPosition, 1.0);
    fragColor = inColor;
}