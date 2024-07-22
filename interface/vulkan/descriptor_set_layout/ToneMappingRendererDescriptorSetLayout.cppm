export module vk_deferred:vulkan.descriptor_set_layout.ToneMappingRendererDescriptorSetLayout;

#ifdef _MSC_VER
import std;
#endif
import vku;

namespace vk_deferred::vulkan::inline descriptor_set_layout {
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
}