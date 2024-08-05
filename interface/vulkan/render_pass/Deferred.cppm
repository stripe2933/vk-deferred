export module vk_deferred:vulkan.rp.Deferred;

#ifdef _MSC_VER
import std;
#endif
import vku;
export import vulkan_hpp;

namespace vk_deferred::vulkan::rp {
    export struct Deferred final : vk::raii::RenderPass {
        explicit Deferred(
            const vk::raii::Device &device [[clang::lifetimebound]]
        ) : RenderPass { device, vk::RenderPassCreateInfo {
            {},
            vku::unsafeProxy({
                vk::AttachmentDescription {
                    {},
                    vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1,
                    // Only stencil test passed fragments will read the input pixel, therefore it doesn't have to be cleared.
                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    {}, {},
                    vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                },
                vk::AttachmentDescription {
                    {},
                    vk::Format::eA2B10G10R10UnormPack32, vk::SampleCountFlagBits::e1,
                    // Only stencil test passed fragments will read the input pixel, therefore it doesn't have to be cleared.
                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    {}, {},
                    vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                },
                vk::AttachmentDescription {
                    {},
                    vk::Format::eD32SfloatS8Uint, vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                },
                vk::AttachmentDescription {
                    {},
                    vk::Format::eB10G11R11UfloatPack32, vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                    {}, {},
                    vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                },
                vk::AttachmentDescription {
                    {},
                    vk::Format::eB8G8R8A8Srgb, vk::SampleCountFlagBits::e1,
                    // All image region will be overwritten by the tone mapping shader using full-quad rendering.
                    vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                    {}, {},
                    vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR,
                },
            }),
            vku::unsafeProxy({
                // G-buffer rendering pass.
                vk::SubpassDescription {
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    {},
                    vku::unsafeProxy({
                        vk::AttachmentReference { 0, vk::ImageLayout::eColorAttachmentOptimal },
                        vk::AttachmentReference { 1, vk::ImageLayout::eColorAttachmentOptimal },
                    }),
                    {},
                    vku::unsafeAddress(vk::AttachmentReference { 2, vk::ImageLayout::eDepthStencilAttachmentOptimal }),
                },
                // Deferred lighting pass.
                vk::SubpassDescription {
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    vku::unsafeProxy({
                        vk::AttachmentReference { 0, vk::ImageLayout::eShaderReadOnlyOptimal },
                        vk::AttachmentReference { 1, vk::ImageLayout::eShaderReadOnlyOptimal },
                    }),
                    vku::unsafeProxy({
                        vk::AttachmentReference { 3, vk::ImageLayout::eColorAttachmentOptimal },
                    }),
                    {},
                    vku::unsafeAddress(vk::AttachmentReference { 2, vk::ImageLayout::eDepthStencilAttachmentOptimal }),
                },
                // Tone mapping pass.
                vk::SubpassDescription {
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    vku::unsafeProxy({
                        vk::AttachmentReference { 3, vk::ImageLayout::eShaderReadOnlyOptimal },
                    }),
                    vku::unsafeProxy({
                        vk::AttachmentReference { 4, vk::ImageLayout::eColorAttachmentOptimal },
                    }),
                },
            }),
            vku::unsafeProxy({
                // Dependency between G-buffer rendering and deferred lighting:
                // Depth/stencil attachment must be written before deferred lighting pipeline reads it.
                // Color attachments must be written before deferred lighting pipeline reads them.
                vk::SubpassDependency {
                    0, 1,
                    vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eFragmentShader,
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eInputAttachmentRead,
                },
                // Dependency between beginning of the render pass and tone mapping:
                // Swapchain image acquirement must be finished before tone mapping pipeline writes it.
                vk::SubpassDependency {
                    vk::SubpassExternal, 2,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    {}, vk::AccessFlagBits::eColorAttachmentWrite,
                },
                // Dependency between deferred lighting and tone mapping:
                // Color attachment must be written before tone mapping pipeline reads it.
                vk::SubpassDependency {
                    1, 2,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
                    vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eInputAttachmentRead,
                },
            }),
        } } { }
    };
}