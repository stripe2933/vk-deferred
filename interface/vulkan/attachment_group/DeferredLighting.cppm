export module vk_deferred:vulkan.ag.DeferredLighting;

export import vku;
export import :vulkan.Gpu;

namespace vk_deferred::vulkan::ag {
    export struct DeferredLighting final : vku::AttachmentGroup {
        DeferredLighting(
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