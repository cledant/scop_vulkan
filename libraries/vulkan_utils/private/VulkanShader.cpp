#include "VulkanShader.hpp"

#include <stdexcept>
#include <cstdio>

std::vector<char>
readFileContent(std::string const &filepath)
{
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Failed to open: " + filepath);
    }

    fseek(file, 0, SEEK_END);
    auto file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<char> file_content(file_size);
    fread(file_content.data(), file_size, 1, file);
    fclose(file);
    return (file_content);
}

VkShaderModule
createShaderModule(VkDevice device,
                   std::vector<char> const &shaderData,
                   std::string const &shaderName)
{
    VkShaderModuleCreateInfo create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shaderData.size();
    create_info.pCode = reinterpret_cast<uint32_t const *>(shaderData.data());

    VkShaderModule shader_module{};
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkShader: Failed to load: " + shaderName);
    }
    return (shader_module);
}

VkShaderModule
loadShader(VkDevice device, const std::string &filepath)
{
    auto shader_code = readFileContent(filepath);
    return (createShaderModule(device, shader_code, filepath));
}
