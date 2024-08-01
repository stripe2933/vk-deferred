export module vk_deferred:vulkan.attachment_group.SwapchainAttachmentGroup;

#ifdef _MSC_VER
import std;
#endif
export import vku;

namespace vk_deferred::vulkan::inline attachment_group {
    export struct SwapchainAttachmentGroup final : vku::AttachmentGroup {
        SwapchainAttachmentGroup(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            const vku::Image &swapchainImage
        ) : AttachmentGroup { extent } {
            addColorAttachment(device, swapchainImage);
        }
    };
}