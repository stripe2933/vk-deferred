export module vk_deferred:vulkan.pipeline.ToneMappingRenderer;

#ifdef _MSC_VER
import std;
#endif
export import vulkan_hpp;
import vku;
export import :vulkan.dsl.HdrInput;
export import :vulkan.rp.Deferred;

namespace vk_deferred::vulkan::inline pipeline {
    export struct ToneMappingRenderer {
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        ToneMappingRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const dsl::HdrInput &descriptorSetLayout [[clang::lifetimebound]],
            const rp::Deferred &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                vku::unsafeProxy(*descriptorSetLayout),
            } },
            pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
#ifdef _MSC_VER
                // TODO: due to the MSVC C++20 module bug, vku::createPipelineStages not works well. Use it instead when fixed.
                vku::unsafeProxy({
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eVertex,
                        *vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/full_triangle.vert.spv", vk::ShaderStageFlagBits::eVertex }.code),
                        } },
                        "main",
                    },
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eFragment,
                        *vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/rec709.frag.spv", vk::ShaderStageFlagBits::eFragment }.code),
                        } },
                        "main",
                    },
                }),
#else
                createPipelineStages(
                    device,
                    vku::Shader { COMPILED_SHADER_DIR "/full_triangle.vert.spv", vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { COMPILED_SHADER_DIR "/rec709.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
#endif
                *pipelineLayout, 1)
                .setRenderPass(*renderPass)
                .setSubpass(2)
            } { }
    };
}