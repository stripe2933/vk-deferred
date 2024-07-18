export module vk_deferred:SharedData;

import std;
import vku;
export import :Gpu;
import :meshes;
export import :pipelines;

export class SharedData {
public:
    // --------------------
    // Swapchain resources.
    // --------------------

    vk::Extent2D swapchainExtent; // Initialized by createSwapchain(const Gpu&, vk::SurfaceKHR).
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;

    // --------------------
    // Render passes.
    // --------------------

    DeferredRenderPass deferredRenderPass;

    // --------------------
    // Pipelines.
    // --------------------

    GBufferRenderer gbufferRenderer;
    DeferredRenderer deferredRenderer;

    // --------------------
    // GPU resources.
    // --------------------

    SphereMesh sphereMesh;
    SphereTransformInstanceBuffer sphereTransforms;
    FloorMesh floorMesh;
    FloorTransformInstanceBuffer floorTransforms;

    explicit SharedData(
        const Gpu &gpu [[clang::lifetimebound]],
        vk::SurfaceKHR surface
    ) : swapchain { createSwapchain(gpu, surface) },
        swapchainImages { (*gpu.device).getSwapchainImagesKHR(*swapchain) },
        deferredRenderPass { gpu.device },
        gbufferRenderer { gpu.device, deferredRenderPass },
        deferredRenderer { gpu.device, deferredRenderPass },
        sphereMesh { gpu.allocator },
        sphereTransforms { gpu.allocator },
        floorMesh { gpu.allocator },
        floorTransforms { gpu.allocator } {
        const vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo {
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            0, // Assume first queue family supports graphics operation.
        } };

        vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queue, [this](vk::CommandBuffer cb) {
            recordAttachmentLayoutInitializationCommands(cb);
        });
        gpu.queue.waitIdle();
    }

private:
    [[nodiscard]] auto createSwapchain(const Gpu &gpu, vk::SurfaceKHR surface) -> vk::raii::SwapchainKHR {
        const vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.physicalDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainExtent = surfaceCapabilities.currentExtent;
        return { gpu.device, vk::SwapchainCreateInfoKHR {
            {},
            surface,
            std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
            vk::Format::eB8G8R8A8Srgb,
            vk::ColorSpaceKHR::eSrgbNonlinear,
            swapchainExtent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive, {},
            surfaceCapabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eFifo,
        } };
    }

    auto recordAttachmentLayoutInitializationCommands(
        vk::CommandBuffer cb
    ) const -> void {
        cb.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
            {},
            {}, {},
            swapchainImages
                | std::views::transform([](vk::Image image) {
                    return vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::ePresentSrcKHR,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        image, vku::fullSubresourceRange(),
                    };
                })
                | std::ranges::to<std::vector>());
    }
};
