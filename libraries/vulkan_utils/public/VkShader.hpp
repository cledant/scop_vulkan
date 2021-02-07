#ifndef SCOP_VULKAN_VKSHADER_HPP
#define SCOP_VULKAN_VKSHADER_HPP

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

std::vector<char> readFileContent(std::string const &filepath);
VkShaderModule createShaderModule(VkDevice device,
                                  std::vector<char> const &shaderData,
                                  std::string const &shaderName);
VkShaderModule loadShader(VkDevice device, std::string const &filepath);

#endif // SCOP_VULKAN_VKSHADER_HPP
