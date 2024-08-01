export module vk_deferred:vulkan.pipeline.GBufferRenderer;

#ifdef _MSC_VER
import std;
#endif
export import glm;
export import vulkan_hpp;
import vku;
export import :vulkan.render_pass.DeferredRenderPass;

namespace vk_deferred::vulkan::inline pipeline {
    export struct GBufferRenderer {
        struct PushConstant {
            glm::mat4 projectionView;
        };

        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        GBufferRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const DeferredRenderPass &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                {},
                vku::unsafeProxy({
                    vk::PushConstantRange { vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstant) },
                }),
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
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/pn_instanced.vert.spv", vk::ShaderStageFlagBits::eVertex }.code),
                        } },
                        "main",
                    },
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eFragment,
                        *vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/gbuffer.frag.spv", vk::ShaderStageFlagBits::eFragment }.code),
                        } },
                        "main",
                    },
                }),
#else
                vku::createPipelineStages(
                    device,
                    vku::Shader { COMPILED_SHADER_DIR "/pn_instanced.vert.spv", vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { COMPILED_SHADER_DIR "/gbuffer.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
#endif
                *pipelineLayout, 2, true)
                .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
                    {},
                    vku::unsafeProxy({
                        vk::VertexInputBindingDescription { 0, sizeof(glm::vec3) * 2, vk::VertexInputRate::eVertex },
                        vk::VertexInputBindingDescription { 1, sizeof(glm::mat4), vk::VertexInputRate::eInstance },
                    }),
                    vku::unsafeProxy({
                        // inPosition
                        vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                        // inNormal
                        vk::VertexInputAttributeDescription { 1, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) },
                        // instanceTransform
                        vk::VertexInputAttributeDescription { 2, 1, vk::Format::eR32G32B32A32Sfloat, 0 },
                        vk::VertexInputAttributeDescription { 3, 1, vk::Format::eR32G32B32A32Sfloat, sizeof(glm::vec4) },
                        vk::VertexInputAttributeDescription { 4, 1, vk::Format::eR32G32B32A32Sfloat, 2 * sizeof(glm::vec4) },
                        vk::VertexInputAttributeDescription { 5, 1, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof(glm::vec4) },
                    }),
                }))
                .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
                    {},
                    true, true, vk::CompareOp::eLess, false,
                    true,
                    // Rendered fragment will be filled with stencil=1.
                    { {}, vk::StencilOp::eReplace, {}, vk::CompareOp::eAlways, 0, 0xFF, 1 },
                }))
                .setRenderPass(*renderPass)
                .setSubpass(0)
            } { }
    };
}