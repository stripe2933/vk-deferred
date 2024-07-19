export module vk_deferred:descriptor_set_layouts;

import vku;

export struct DeferredLightRendererDescriptorSetLayout final : vku::DescriptorSetLayouts<2> {
    explicit DeferredLightRendererDescriptorSetLayout(
        const vk::raii::Device &device [[clang::lifetimebound]]
    ) : DescriptorSetLayouts {
            device,
            vk::DescriptorSetLayoutCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
                    vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
                }),
            },
        } { }
};

export struct ToneMappingRendererDescriptorSetLayout final : vku::DescriptorSetLayouts<1> {
    explicit ToneMappingRendererDescriptorSetLayout(
        const vk::raii::Device &device [[clang::lifetimebound]]
    ) : DescriptorSetLayouts {
            device,
            vk::DescriptorSetLayoutCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment },
                }),
            },
        } { }
};