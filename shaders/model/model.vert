#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inVertexPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;
layout(location = 5) in mat4 instanceMatrix;

layout(location = 0) out vec2 outFragTexCoord;

layout(binding = 0) uniform SystemUBO {
    mat4 view_proj;
} systemUbo;

void main() {
    gl_Position = systemUbo.view_proj * instanceMatrix * vec4(inVertexPosition, 1.0);
    outFragTexCoord = inTexCoord;
}