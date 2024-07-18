export module vk_deferred:render_passes;

import vku;
export import vulkan_hpp;

export struct DeferredRenderPass final : vk::raii::RenderPass {
    explicit DeferredRenderPass(
        const vk::raii::Device &device [[clang::lifetimebound]]
    ) : RenderPass { device, vk::RenderPassCreateInfo {
        {},
        vku::unsafeProxy({
            vk::AttachmentDescription {
                {},
                vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                {}, {},
                vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
            },
            vk::AttachmentDescription {
                {},
                vk::Format::eA2B10G10R10UnormPack32, vk::SampleCountFlagBits::e1,
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
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                {}, {},
                vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::ePresentSrcKHR,
            },
        }),
        vku::unsafeProxy({
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
            vk::SubpassDependency {
                0, 1,
                vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eFragmentShader,
                vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eInputAttachmentRead,
            },
            vk::SubpassDependency {
                vk::SubpassExternal, 2,
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {}, vk::AccessFlagBits::eColorAttachmentWrite,
            },
            vk::SubpassDependency {
                1, 2,
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
                vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eInputAttachmentRead,
            },
        }),
    } } { }
};