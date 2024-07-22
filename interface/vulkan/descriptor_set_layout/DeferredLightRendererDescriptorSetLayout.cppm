export module vk_deferred:vulkan.descriptor_set_layout.DeferredLightRendererDescriptorSetLayout;

#ifdef _MSC_VER
import std;
#endif
import vku;

namespace vk_deferred::vulkan::inline descriptor_set_layout {
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
}