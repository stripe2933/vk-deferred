export module vk_deferred:vulkan.ag.Swapchain;

import std;
export import vku;

namespace vk_deferred::vulkan::ag {
    export struct Swapchain final : vku::AttachmentGroup {
        Swapchain(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            std::span<const vk::Image> swapchainImages
        ) : AttachmentGroup { extent } {
            addSwapchainAttachment(device, swapchainImages, vk::Format::eB8G8R8A8Srgb);
        }
    };
}