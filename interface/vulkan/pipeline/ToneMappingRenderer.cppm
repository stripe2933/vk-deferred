export module vk_deferred:vulkan.pipeline.ToneMappingRenderer;

#ifdef _MSC_VER
import std;
#endif
export import vulkan_hpp;
import vku;
export import :vulkan.descriptor_set_layout.ToneMappingRendererDescriptorSetLayout;
export import :vulkan.render_pass.DeferredRenderPass;

namespace vk_deferred::vulkan::inline pipeline {
    export struct ToneMappingRenderer {
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        ToneMappingRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const ToneMappingRendererDescriptorSetLayout &descriptorSetLayout [[clang::lifetimebound]],
            const DeferredRenderPass &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                vku::unsafeProxy(descriptorSetLayout.getHandles()),
            } },
            pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
                vku::createPipelineStages(
                    device,
                    vku::Shader { COMPILED_SHADER_DIR "/full_triangle.vert.spv", vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { COMPILED_SHADER_DIR "/rec709.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
                *pipelineLayout, 1)
                .setRenderPass(*renderPass)
                .setSubpass(2)
            } { }
    };
}