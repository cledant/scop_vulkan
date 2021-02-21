#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {

    vec4 tex_color = texture(texSampler, fragTexCoord);
    if (tex_color.a < 0.5f) {
        discard;
    }
    outColor = vec4(tex_color.rgb, 1.0f);
}