#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform ModelUBO {
    vec3 diffuse_color;
} modelUbo;
layout(binding = 2) uniform sampler2D texSampler;

void main() {

    vec4 tex_color = texture(texSampler, inFragTexCoord);
    if (tex_color.a < 0.5f) {
        discard;
    }
    outColor = vec4(tex_color.rgb * modelUbo.diffuse_color.rgb, 1.0f);
}