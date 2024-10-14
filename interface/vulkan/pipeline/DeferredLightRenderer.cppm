export module vk_deferred:vulkan.pipeline.DeferredLightingRenderer;

#ifdef _MSC_VER
import std;
#endif
export import glm;
export import vulkan_hpp;
import vku;
export import :vulkan.dsl.GBufferInput;
export import :vulkan.rp.Deferred;

namespace vk_deferred::vulkan::inline pipeline {
    export struct DeferredLightingRenderer {
        struct PushConstant {
            glm::mat4 projectionView;
            glm::vec3 viewPosition;
        };

        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        DeferredLightingRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const dsl::GBufferInput &descriptorSetLayout [[clang::lifetimebound]],
            const rp::Deferred &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                vku::unsafeProxy(*descriptorSetLayout),
                vku::unsafeProxy({
                    vk::PushConstantRange { vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(PushConstant) },
                }),
            } },
            pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
                createPipelineStages(
                    device,
                    vku::Shader::fromSpirvFile(COMPILED_SHADER_DIR "/light_volume.vert.spv", vk::ShaderStageFlagBits::eVertex),
                    vku::Shader::fromSpirvFile(COMPILED_SHADER_DIR "/deferred_lighting.frag.spv", vk::ShaderStageFlagBits::eFragment)).get(),
                *pipelineLayout, 1, true)
                .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
                    {},
                    vku::unsafeProxy({
                        vk::VertexInputBindingDescription { 0, sizeof(glm::vec3) * 2, vk::VertexInputRate::eVertex },
                        vk::VertexInputBindingDescription { 1, sizeof(glm::vec3) + sizeof(float) + sizeof(glm::vec3), vk::VertexInputRate::eInstance },
                    }),
                    vku::unsafeProxy({
                        // inPosition
                        vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                        // inInstancePosition
                        vk::VertexInputAttributeDescription { 1, 1, vk::Format::eR32G32B32Sfloat, 0 },
                        // inInstanceRadius
                        vk::VertexInputAttributeDescription { 2, 1, vk::Format::eR32Sfloat, sizeof(glm::vec3) },
                        // inInstanceColor
                        vk::VertexInputAttributeDescription { 3, 1, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) + sizeof(float) },
                    }),
                }))
                .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
                    {},
                    true, false, vk::CompareOp::eLess, false,
                    // Only stencil=1 (which indicates the fragment is rendered by GBufferRenderer) will be processed.
                    true, { {}, {}, {}, vk::CompareOp::eEqual, 0xFF, 0, 1 },
                }))
                .setPColorBlendState(vku::unsafeAddress(vk::PipelineColorBlendStateCreateInfo {
                    {},
                    false, {},
                    vku::unsafeProxy({
                        // Use add blending to accumulate light.
                        vk::PipelineColorBlendAttachmentState {
                            true,
                            vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                            {}, {}, {},
                            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB,
                        },
                    }),
                }))
                .setRenderPass(*renderPass)
                .setSubpass(1)
            } { }
    };
}