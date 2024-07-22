export module vk_deferred:vulkan.attachment_group.GBufferAttachmentGroup;

import vku;
export import :vulkan.Gpu;

namespace vk_deferred::vulkan::inline attachment_group {
    export struct GBufferAttachmentGroup final : vku::AttachmentGroup {
        GBufferAttachmentGroup(
            const Gpu &gpu [[clang::lifetimebound]],
            const vk::Extent2D &extent
        ) : AttachmentGroup { extent } {
            // Position.
            addColorAttachment(gpu.device, storeImage(createColorImage(
                gpu.allocator,
                vk::Format::eR32G32B32A32Sfloat, // Alpha component will not be in use.
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
                vma::AllocationCreateInfo {
                    {},
                    vma::MemoryUsage::eAutoPreferDevice,
                    {},
                    vk::MemoryPropertyFlagBits::eLazilyAllocated,
                })));
            // Normal.
            addColorAttachment(gpu.device, storeImage(createColorImage(
                gpu.allocator,
                vk::Format::eA2B10G10R10UnormPack32, // [-1, 1] -> [0, 1] projected. Alpha component will not be in use.
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
                vma::AllocationCreateInfo {
                    {},
                    vma::MemoryUsage::eAutoPreferDevice,
                    {},
                    vk::MemoryPropertyFlagBits::eLazilyAllocated,
                })));
            // Depth/stencil (drawn fragment will filled with stencil=1).
            setDepthStencilAttachment(gpu.device, storeImage(createDepthStencilImage(
                gpu.allocator,
                vk::Format::eD32SfloatS8Uint,
                vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
                // It seems MoltenVK has bug that set vk::MemoryPropertyFlagBits::eLazilyAllocated makes
                //  - 1st subpass storeOp=DontCare, which must be Store.
                //  - 1st subpass stencilStoreOp=DontCare, which must be Store.
                //  - 2nd subpass loadOp=DontCare, which must be Load.
                //  - 2nd subpass stencilLoadOp=DontCare, which must be Load.
                // TODO: Check if the same issue occurs on other platforms.
                vma::AllocationCreateInfo { {}, vma::MemoryUsage::eAutoPreferDevice })));
        }
    };
}