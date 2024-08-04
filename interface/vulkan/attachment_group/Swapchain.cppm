export module vk_deferred:vulkan.ag.Swapchain;

#ifdef _MSC_VER
import std;
#endif
export import vku;

namespace vk_deferred::vulkan::ag {
    export struct Swapchain final : vku::AttachmentGroup {
        Swapchain(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            const vku::Image &swapchainImage
        ) : AttachmentGroup { extent } {
            addColorAttachment(device, swapchainImage);
        }
    };
}