export module vk_deferred:descriptor_set_layouts;

#ifdef _MSC_VER
import std;
#endif
import vku;

export struct DeferredLightRendererDescriptorSetLayout final : vku::DescriptorSetLayouts<2> {
    explicit DeferredLightRendererDescriptorSetLayout(
        const vk::raii::Device &device [[clang::lifetimebound]]
    ) : DescriptorSetLayouts {
            device,
            vk::DescriptorSetLayoutCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment }, // Position.
                    vk::DescriptorSetLayoutBinding { 1, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment }, // [0, 1] bounded normal.
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
                    vk::DescriptorSetLayoutBinding { 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment }, // HDR image.
                }),
            },
        } { }
};