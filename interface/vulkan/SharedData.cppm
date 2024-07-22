export module vk_deferred:vulkan.SharedData;

import std;
import vku;
import :vulkan.buffer.FloorTransforms;
import :vulkan.buffer.SphereTransforms;
export import :vulkan.Gpu;
import :vulkan.mesh.Floor;
import :vulkan.mesh.Sphere;
export import :vulkan.pipeline.GBufferRenderer;
export import :vulkan.pipeline.DeferredLightingRenderer;
export import :vulkan.pipeline.ToneMappingRenderer;

namespace vk_deferred::vulkan {
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
        // Descriptor set layouts.
        // --------------------

        DeferredLightRendererDescriptorSetLayout deferredLightRendererDescriptorSetLayout;
        ToneMappingRendererDescriptorSetLayout toneMappingRendererDescriptorSetLayout;

        // --------------------
        // Pipelines.
        // --------------------

        GBufferRenderer gbufferRenderer;
        DeferredLightingRenderer deferredLightingRenderer;
        ToneMappingRenderer toneMappingRenderer;

        // --------------------
        // GPU resources.
        // --------------------

        mesh::Sphere sphereMesh;
        buffer::SphereTransforms sphereTransforms;
        mesh::Floor floorMesh;
        buffer::FloorTransforms floorTransforms;

        explicit SharedData(
            const Gpu &gpu [[clang::lifetimebound]],
            vk::SurfaceKHR surface
        ) : swapchain { createSwapchain(gpu, surface) },
            swapchainImages { (*gpu.device).getSwapchainImagesKHR(*swapchain) },
            deferredRenderPass { gpu.device },
            deferredLightRendererDescriptorSetLayout { gpu.device },
            toneMappingRendererDescriptorSetLayout { gpu.device },
            gbufferRenderer { gpu.device, deferredRenderPass },
            deferredLightingRenderer { gpu.device, deferredLightRendererDescriptorSetLayout, deferredRenderPass },
            toneMappingRenderer { gpu.device, toneMappingRendererDescriptorSetLayout, deferredRenderPass },
            sphereMesh { gpu.allocator },
            sphereTransforms { gpu.allocator },
            floorMesh { gpu.allocator },
            floorTransforms { gpu.allocator } {
            const vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo {
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                gpu.queueFamilies.graphicsPresent,
            } };

            vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queues.graphicsPresent, [this](vk::CommandBuffer cb) {
                recordAttachmentLayoutInitializationCommands(cb);
            });
            gpu.queues.graphicsPresent.waitIdle();
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
            // Initialize the image layouts as desired end layout of the frame, for avoid the undefined layout for
            // initialLayout in render passes.
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
}