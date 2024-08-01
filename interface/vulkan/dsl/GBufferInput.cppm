export module vk_deferred:vulkan.dsl.GBufferInput;

#ifdef _MSC_VER
import std;
#endif
import vku;

namespace vk_deferred::vulkan::dsl {
    export struct GBufferInput final : vku::DescriptorSetLayout<vk::DescriptorType::eInputAttachment, vk::DescriptorType::eInputAttachment> {
        explicit GBufferInput(
            const vk::raii::Device &device [[clang::lifetimebound]]
        ) : DescriptorSetLayout {
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