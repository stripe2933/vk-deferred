export module vk_deferred:vulkan.attachment_group.DeferredLightingAttachmentGroup;

export import vku;
export import :vulkan.Gpu;

namespace vk_deferred::vulkan::inline attachment_group {
    export struct DeferredLightingAttachmentGroup final : vku::AttachmentGroup {
        DeferredLightingAttachmentGroup(
            const Gpu &gpu [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            const vku::Image &depthStencilImage [[clang::lifetimebound]]
        ) : AttachmentGroup { extent } {
            addColorAttachment(gpu.device, storeImage(createColorImage(
                gpu.allocator,
                vk::Format::eB10G11R11UfloatPack32, // Use high-bit floats to represent HDR.
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
                vku::allocation::deviceLocalTransient)));
            setDepthStencilAttachment(gpu.device, depthStencilImage);
        }
    };
}