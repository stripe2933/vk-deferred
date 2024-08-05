module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_deferred:vulkan.Frame;

import std;
import vku;
import :vulkan.buffer.LightInstances;
export import :vulkan.Gpu;
export import :vulkan.SharedData;
import :vulkan.ag.GBuffer;
import :vulkan.ag.DeferredLighting;
import :vulkan.ag.Swapchain;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

template <typename... Arrays>
[[nodiscard]] constexpr auto array_cat(Arrays &&...arrays) {
    return std::apply([](auto &&...xs) {
        return std::array { FWD(xs)... };
    }, std::tuple_cat(FWD(arrays)...));
}

namespace vk_deferred::vulkan {
    export class Frame {
    public:
        Frame(
            const Gpu &gpu [[clang::lifetimebound]],
            const SharedData &sharedData [[clang::lifetimebound]],
            std::uint32_t seed
        ) : gpu { gpu },
            sharedData { sharedData },
            lightInstanceBuffer { gpu.allocator, std::mt19937 { seed } } {
            std::tie(gBufferInputDescriptorSet, hdrImageDescriptorSet)
                = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(
                    sharedData.gBufferInputDescriptorSetLayout,
                    sharedData.hdrInputDescriptorSetLayout));

            // Update per-frame descriptors.
            gpu.device.updateDescriptorSets({
                gBufferInputDescriptorSet.getWrite<0>(vku::unsafeProxy({
                    vk::DescriptorImageInfo { {}, *gbufferAttachmentGroup.colorAttachments[0].view, vk::ImageLayout::eShaderReadOnlyOptimal },
                })),
                gBufferInputDescriptorSet.getWrite<1>(vku::unsafeProxy({
                    vk::DescriptorImageInfo { {}, *gbufferAttachmentGroup.colorAttachments[1].view, vk::ImageLayout::eShaderReadOnlyOptimal },
                })),
                hdrImageDescriptorSet.getWrite<0>(vku::unsafeProxy({
                    vk::DescriptorImageInfo { {}, *deferredLightingAttachmentGroup.colorAttachments[0].view, vk::ImageLayout::eShaderReadOnlyOptimal },
                })),
            }, {});

            // Initialize attachment layouts.
            vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queues.graphicsPresent, [this](vk::CommandBuffer cb) {
                recordAttachmentLayoutInitializationCommands(cb);
            });
            gpu.queues.graphicsPresent.waitIdle();
        }

        auto onLoop(
            float time
        ) -> void {
            // Wait for the previous frame to finish.
            const vk::Result frameFinishResult = gpu.device.waitForFences(*frameFinishFence, true, ~0U);
            assert(frameFinishResult == vk::Result::eSuccess && "Failed to wait for frame finish fence.");
            gpu.device.resetFences(*frameFinishFence);

            constexpr glm::vec3 eye { 10.f, 10.f, -10.f };
            const glm::mat4 projectionView
                = glm::perspective(glm::radians(45.f), vku::aspect(sharedData.swapchainExtent), 0.1f, 100.f)
                * lookAt(eye, glm::vec3 { 0.f }, { 0.f, 1.f, 0.f });

            lightInstanceBuffer.update(time);

            // Acquire swapchain image to render.
            const auto [swapchainImageAvailableResult, swapchainImageIndex] = (*gpu.device).acquireNextImageKHR(*sharedData.swapchain, ~0U, *swapchainImageAvailableSemaphore);
            assert(swapchainImageAvailableResult == vk::Result::eSuccess && "Failed to acquire next swapchain image.");

            commandPool.reset();
            commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

            // Set viewport and scissor, which are pipeline dynamic states.
            commandBuffer.setViewport(0, gbufferAttachmentGroup.getViewport(true));
            commandBuffer.setScissor(0, gbufferAttachmentGroup.getScissor());

            commandBuffer.beginRenderPass(vk::RenderPassBeginInfo {
                *sharedData.deferredRenderPass,
                *framebuffers[swapchainImageIndex],
                vk::Rect2D { { 0, 0 }, sharedData.swapchainExtent },
                vku::unsafeProxy<vk::ClearValue>({
                    vk::ClearColorValue{},
                    vk::ClearColorValue{},
                    vk::ClearDepthStencilValue { 1.f, 0 },
                    vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                    vk::ClearColorValue{},
                }),
            }, vk::SubpassContents::eInline);

            // 1st subpass: G-buffer construction
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.gbufferRenderer.pipeline);
            commandBuffer.pushConstants<GBufferRenderer::PushConstant>(*sharedData.gbufferRenderer.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, GBufferRenderer::PushConstant {
                projectionView,
            });

            // Draw spheres.
            commandBuffer.bindIndexBuffer(sharedData.sphereMesh.indexBuffer, 0, vk::IndexType::eUint16);
            commandBuffer.bindVertexBuffers(0, { sharedData.sphereMesh.vertexBuffer.buffer, sharedData.sphereTransforms.buffer }, { 0, 0 });
            commandBuffer.drawIndexed(sharedData.sphereMesh.drawCount, sharedData.sphereTransforms.instanceCount, 0, 0, 0);

            // Draw floor.
            commandBuffer.bindVertexBuffers(0, { sharedData.floorMesh.vertexBuffer.buffer, sharedData.floorTransforms.buffer }, { 0, 0 });
            commandBuffer.draw(sharedData.floorMesh.drawCount, sharedData.floorTransforms.instanceCount, 0, 0);

            commandBuffer.nextSubpass(vk::SubpassContents::eInline);

            // 2nd subpass: lighting.
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.deferredLightingRenderer.pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *sharedData.deferredLightingRenderer.pipelineLayout, 0, gBufferInputDescriptorSet, {});
            commandBuffer.pushConstants<DeferredLightingRenderer::PushConstant>(*sharedData.deferredLightingRenderer.pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, DeferredLightingRenderer::PushConstant {
                projectionView,
                eye,
            });

            // Draw light volumes.
            commandBuffer.bindIndexBuffer(sharedData.sphereMesh.indexBuffer, 0, vk::IndexType::eUint16);
            commandBuffer.bindVertexBuffers(0, { sharedData.sphereMesh.vertexBuffer.buffer, lightInstanceBuffer.buffer }, { 0, 0 });
            commandBuffer.drawIndexed(sharedData.sphereMesh.drawCount, lightInstanceBuffer.instanceCount, 0, 0, 0);

            commandBuffer.nextSubpass(vk::SubpassContents::eInline);

            // 3rd subpass: tone mapping.
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.toneMappingRenderer.pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *sharedData.toneMappingRenderer.pipelineLayout, 0, hdrImageDescriptorSet, {});
            commandBuffer.draw(3, 1, 0, 0);

            commandBuffer.endRenderPass();

            commandBuffer.end();

            // Submit commandBuffer to queue.
            gpu.queues.graphicsPresent.submit(vk::SubmitInfo {
                *swapchainImageAvailableSemaphore,
                vku::unsafeProxy({ vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput } }),
                commandBuffer,
                *drawFinishSemaphore,
            }, *frameFinishFence);

            // Present swapchain image.
            const vk::Result swapchainImagePresentResult = gpu.queues.graphicsPresent.presentKHR(vk::PresentInfoKHR {
                *drawFinishSemaphore,
                *sharedData.swapchain,
                swapchainImageIndex,
            });
            assert(swapchainImagePresentResult == vk::Result::eSuccess && "Failed to present swapchain image.");
        }

    private:
        const Gpu &gpu;
        const SharedData &sharedData;

        // --------------------
        // Frame-exclusive Attachment groups.
        // --------------------

        ag::GBuffer gbufferAttachmentGroup { gpu, sharedData.swapchainExtent };
        ag::DeferredLighting deferredLightingAttachmentGroup { gpu, sharedData.swapchainExtent, gbufferAttachmentGroup.depthStencilAttachment->image };
        std::vector<ag::Swapchain> swapchainAttachmentGroups = createSwapchainAttachmentGroups();

        // --------------------
        // GPU resources.
        // --------------------

        buffer::LightInstances lightInstanceBuffer;

        // --------------------
        // Framebuffers.
        // --------------------

        std::vector<vk::raii::Framebuffer> framebuffers = createFramebuffers();

        // --------------------
        // Descriptor pools and sets.
        // --------------------

        vk::raii::DescriptorPool descriptorPool = createDescriptorPool();
        vku::DescriptorSet<dsl::GBufferInput> gBufferInputDescriptorSet;
        vku::DescriptorSet<dsl::HdrInput> hdrImageDescriptorSet;

        // --------------------
        // Command pools and buffers.
        // --------------------

        vk::raii::CommandPool commandPool = createCommandPool();
        vk::CommandBuffer commandBuffer = (*gpu.device).allocateCommandBuffers(vk::CommandBufferAllocateInfo {
            *commandPool,
            vk::CommandBufferLevel::ePrimary,
            1,
        })[0];

        // --------------------
        // Synchronization stuffs.
        // --------------------

        vk::raii::Semaphore swapchainImageAvailableSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Semaphore drawFinishSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Fence frameFinishFence { gpu.device, vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled } };

        [[nodiscard]] auto createSwapchainAttachmentGroups() const -> std::vector<ag::Swapchain> {
            return sharedData.swapchainImages
                | std::views::transform([&](vk::Image image) {
                    return ag::Swapchain {
                        gpu.device,
                        sharedData.swapchainExtent,
                        vku::Image { image, vk::Extent3D { sharedData.swapchainExtent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 },
                    };
                })
                | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto createFramebuffers() const -> std::vector<vk::raii::Framebuffer> {
            return swapchainAttachmentGroups
                | std::views::transform([&](const auto &swapchainAttachmentGroup) {
                    return vk::raii::Framebuffer { gpu.device, vk::FramebufferCreateInfo {
                        {},
                        *sharedData.deferredRenderPass,
                        vku::unsafeProxy({
                            *gbufferAttachmentGroup.colorAttachments[0].view,
                            *gbufferAttachmentGroup.colorAttachments[1].view,
                            *gbufferAttachmentGroup.depthStencilAttachment->view,
                            *deferredLightingAttachmentGroup.colorAttachments[0].view,
                            *swapchainAttachmentGroup.colorAttachments[0].view,
                        }),
                        sharedData.swapchainExtent.width, sharedData.swapchainExtent.height, 1,
                    } };
                })
                | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto createDescriptorPool() const -> vk::raii::DescriptorPool {
            return { gpu.device, getPoolSizes(
                sharedData.gBufferInputDescriptorSetLayout,
                sharedData.hdrInputDescriptorSetLayout).getDescriptorPoolCreateInfo() };
        }

        [[nodiscard]] auto createCommandPool() const -> vk::raii::CommandPool {
            return { gpu.device, vk::CommandPoolCreateInfo {
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                gpu.queueFamilies.graphicsPresent,
            } };
        }

        auto recordAttachmentLayoutInitializationCommands(
            vk::CommandBuffer cb
        ) const -> void {
            // Initialize the image layouts as desired end layout of the frame, for avoid the undefined layout for
            // initialLayout in render passes.
            cb.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {},
                {
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        gbufferAttachmentGroup.colorAttachments[0].image, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        gbufferAttachmentGroup.colorAttachments[1].image, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        gbufferAttachmentGroup.depthStencilAttachment->image, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        deferredLightingAttachmentGroup.colorAttachments[0].image, vku::fullSubresourceRange(),
                    }
                });
        }
    };
}