export module vk_deferred:pipelines;

export import glm;
export import vulkan_hpp;
import vku;
export import :render_passes;

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
            vku::createPipelineStages(
                device,
                vku::Shader { COMPILED_SHADER_DIR "/pn_instanced.vert.spv", vk::ShaderStageFlagBits::eVertex },
                vku::Shader { COMPILED_SHADER_DIR "/gbuffer.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
            *pipelineLayout, 2, true)
            .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::VertexInputBindingDescription { 0, sizeof(glm::vec3) * 2, vk::VertexInputRate::eVertex },
                    vk::VertexInputBindingDescription { 1, sizeof(glm::mat4), vk::VertexInputRate::eInstance },
                }),
                vku::unsafeProxy({
                    vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                    vk::VertexInputAttributeDescription { 1, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) },
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
                { {}, vk::StencilOp::eReplace, {}, vk::CompareOp::eAlways, 0, 0xFF, 1 },
            }))
            .setRenderPass(*renderPass)
            .setSubpass(0)
        } { }
};

export struct DeferredLightingRenderer {
    struct PushConstant {
        glm::mat4 projectionView;
        glm::vec3 viewPosition;
    };

    vk::raii::DescriptorSetLayout descriptorSetLayout;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    DeferredLightingRenderer(
        const vk::raii::Device &device [[clang::lifetimebound]],
        const DeferredRenderPass &renderPass [[clang::lifetimebound]]
    ) : descriptorSetLayout { device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy({
                vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
                vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
            }),
        } },
        pipelineLayout { device, vk::PipelineLayoutCreateInfo {
            {},
            *descriptorSetLayout,
            vku::unsafeProxy({
                vk::PushConstantRange { vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(PushConstant) },
            }),
        } },
        pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
            vku::createPipelineStages(
                device,
                vku::Shader { COMPILED_SHADER_DIR "/light_volume.vert.spv", vk::ShaderStageFlagBits::eVertex },
                vku::Shader { COMPILED_SHADER_DIR "/deferred_lighting.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
            *pipelineLayout, 1, true)
            .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::VertexInputBindingDescription { 0, sizeof(glm::vec3), vk::VertexInputRate::eVertex },
                    vk::VertexInputBindingDescription { 1, sizeof(glm::vec3) + sizeof(float) + sizeof(glm::vec3), vk::VertexInputRate::eInstance },
                }),
                vku::unsafeProxy({
                    vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                    vk::VertexInputAttributeDescription { 1, 1, vk::Format::eR32G32B32Sfloat, 0 },
                    vk::VertexInputAttributeDescription { 2, 1, vk::Format::eR32Sfloat, sizeof(glm::vec3) },
                    vk::VertexInputAttributeDescription { 3, 1, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) + sizeof(float) },
                }),
            }))
            .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
                {},
                true, false, vk::CompareOp::eLess, false,
                true, { {}, {}, {}, vk::CompareOp::eEqual, 0xFF, 0, 1 },
            }))
            .setPColorBlendState(vku::unsafeAddress(vk::PipelineColorBlendStateCreateInfo {
                {},
                false, {},
                vku::unsafeProxy({
                    vk::PipelineColorBlendAttachmentState {
                        true,
                        vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                        vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                    },
                }),
            }))
            .setRenderPass(*renderPass)
            .setSubpass(1)
        } { }
};

export struct ToneMappingRenderer {
    vk::raii::DescriptorSetLayout descriptorSetLayout;
    vk::raii::PipelineLayout pipelineLayout;
    vk::raii::Pipeline pipeline;

    ToneMappingRenderer(
        const vk::raii::Device &device [[clang::lifetimebound]],
        const DeferredRenderPass &renderPass [[clang::lifetimebound]]
    ) : descriptorSetLayout { device, vk::DescriptorSetLayoutCreateInfo {
            {},
            vku::unsafeProxy({
                vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
            }),
        } },
        pipelineLayout { device, vk::PipelineLayoutCreateInfo {
            {},
            *descriptorSetLayout,
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