export module vk_deferred:vulkan.dsl.HdrInput;

#ifdef _MSC_VER
import std;
#endif
import vku;

namespace vk_deferred::vulkan::dsl {
    export struct HdrInput final : vku::DescriptorSetLayout<vk::DescriptorType::eInputAttachment> {
        explicit HdrInput(
            const vk::raii::Device &device [[clang::lifetimebound]]
        ) : DescriptorSetLayout {
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