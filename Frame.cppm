module;

#include <cassert>

export module vk_deferred:Frame;

import std;
export import :Gpu;
export import :SharedData;
import :attachment_groups;
import :meshes;

export class Frame {
public:
    Frame(
        const Gpu &gpu [[clang::lifetimebound]],
        const SharedData &sharedData [[clang::lifetimebound]]
    ) : gpu { gpu },
        sharedData { sharedData },
        lightInstanceBuffer { gpu.allocator, std::mt19937 { std::random_device{}() } } {
        // Update per-frame descriptors.
        gpu.device.updateDescriptorSets({
            vk::WriteDescriptorSet {
                descriptorSet,
                0,
                0,
                vk::DescriptorType::eInputAttachment,
                vku::unsafeProxy({ vk::DescriptorImageInfo { {}, *gbufferAttachmentGroup.colorAttachments[0].view, vk::ImageLayout::eShaderReadOnlyOptimal } }),
            },
            vk::WriteDescriptorSet {
                descriptorSet,
                1,
                0,
                vk::DescriptorType::eInputAttachment,
                vku::unsafeProxy({ vk::DescriptorImageInfo { {}, *gbufferAttachmentGroup.colorAttachments[1].view, vk::ImageLayout::eShaderReadOnlyOptimal } }),
            },
        }, {});

        // Initialize attachment layouts.
        vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queue, [this](vk::CommandBuffer cb) {
            recordAttachmentLayoutInitializationCommands(cb);
        });
        gpu.queue.waitIdle();
    }

    auto onLoop() -> void {
        static std::uint64_t counter = 0;
        ++counter;

        // Wait for the previous frame to finish.
        const vk::Result frameFinishResult = gpu.device.waitForFences(*frameFinishFence, true, ~0U);
        assert(frameFinishResult == vk::Result::eSuccess && "Failed to wait for frame finish fence.");
        gpu.device.resetFences(*frameFinishFence);

        constexpr glm::vec3 eye { 10.f, 10.f, -10.f };
        const glm::mat4 projectionView
            = glm::perspective(glm::radians(45.f), static_cast<float>(sharedData.swapchainExtent.width) / sharedData.swapchainExtent.height, 0.1f, 100.f)
            * lookAt(eye, glm::vec3 { 0.f }, { 0.f, 1.f, 0.f });

        lightInstanceBuffer.update(counter / 120.f);

        // Acquire swapchain image to render.
        const auto [swapchainImageAvailableResult, swapchainImageIndex] = (*gpu.device).acquireNextImageKHR(*sharedData.swapchain, ~0U, *swapchainImageAvailableSemaphore, {});
        assert(swapchainImageAvailableResult == vk::Result::eSuccess && "Failed to acquire next swapchain image.");

        commandPool.reset();
        commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        // Set viewport and scissor, which are pipeline dynamic states.
        gbufferAttachmentGroup.setViewport(commandBuffer, true);
        gbufferAttachmentGroup.setScissor(commandBuffer);

        commandBuffer.beginRenderPass(vk::RenderPassBeginInfo {
            *sharedData.deferredRenderPass,
            *framebuffers[swapchainImageIndex],
            vk::Rect2D { { 0, 0 }, sharedData.swapchainExtent },
            vku::unsafeProxy<vk::ClearValue>({
                vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                vk::ClearDepthStencilValue { 1.f, 0 },
                vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
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
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.deferredRenderer.pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *
            sharedData.deferredRenderer.pipelineLayout, 0, descriptorSet, {});
        commandBuffer.pushConstants<DeferredRenderer::PushConstant>(*sharedData.deferredRenderer.pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, DeferredRenderer::PushConstant {
            projectionView,
            eye,
        });

        // Draw light volumes.
        commandBuffer.bindIndexBuffer(sharedData.sphereMesh.indexBuffer, 0, vk::IndexType::eUint16);
        commandBuffer.bindVertexBuffers(0, { sharedData.sphereMesh.vertexBuffer.buffer, lightInstanceBuffer.buffer }, { 0, 0 });
        for (int i = 0; i < lightInstanceBuffer.instanceCount; ++i) {
            commandBuffer.drawIndexed(sharedData.sphereMesh.drawCount, 1, 0, 0, i);
        }

        commandBuffer.endRenderPass();

        commandBuffer.end();

        // Submit commandBuffer to queue.
        gpu.queue.submit(vk::SubmitInfo {
            *swapchainImageAvailableSemaphore,
            vku::unsafeProxy({ vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput } }),
            commandBuffer,
            *drawFinishSemaphore,
        }, *frameFinishFence);

        // Present swapchain image.
        const vk::Result swapchainImagePresentResult = gpu.queue.presentKHR(vk::PresentInfoKHR {
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

    GBufferAttachmentGroup gbufferAttachmentGroup { gpu, sharedData.swapchainExtent };
    std::vector<SwapchainAttachmentGroup> swapchainAttachmentGroups = createSwapchainAttachmentGroups();

    // --------------------
    // GPU resources.
    // --------------------

    LightInstanceBuffer lightInstanceBuffer;

    // --------------------
    // Framebuffers.
    // --------------------

    std::vector<vk::raii::Framebuffer> framebuffers = createFramebuffers();

    // --------------------
    // Descriptor pools and sets.
    // --------------------

    vk::raii::DescriptorPool descriptorPool = createDescriptorPool();
    vk::DescriptorSet descriptorSet = (*gpu.device).allocateDescriptorSets({
        *descriptorPool,
        *sharedData.deferredRenderer.descriptorSetLayout,
    })[0];

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

    [[nodiscard]] auto createSwapchainAttachmentGroups() const -> std::vector<SwapchainAttachmentGroup> {
        return sharedData.swapchainImages
            | std::views::transform([&](vk::Image image) {
                return SwapchainAttachmentGroup {
                    gpu.device,
                    sharedData.swapchainExtent,
                    vku::Image { image, vk::Extent3D { sharedData.swapchainExtent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 },
                    gbufferAttachmentGroup.depthStencilAttachment->image,
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
                        *swapchainAttachmentGroup.colorAttachments[0].view,
                    }),
                    sharedData.swapchainExtent.width, sharedData.swapchainExtent.height, 1,
                } };
            })
            | std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto createDescriptorPool() const -> vk::raii::DescriptorPool {
        return { gpu.device, vk::DescriptorPoolCreateInfo {
            {},
            1,
            vku::unsafeProxy({
                vk::DescriptorPoolSize { vk::DescriptorType::eInputAttachment, 2 },
            }),
        } };
    }

    [[nodiscard]] auto createCommandPool() const -> vk::raii::CommandPool {
        return { gpu.device, vk::CommandPoolCreateInfo {
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            0, // Assume first queue family supports graphics operation.
        } };
    }

    auto recordAttachmentLayoutInitializationCommands(
        vk::CommandBuffer cb
    ) const -> void {
        std::vector<vk::ImageMemoryBarrier> barriers;
        barriers.append_range(gbufferAttachmentGroup.colorAttachments | std::views::transform([](const auto &attachment) {
            return vk::ImageMemoryBarrier {
                {}, {},
                {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                attachment.image, vku::fullSubresourceRange(),
            };
        }));
        barriers.push_back(vk::ImageMemoryBarrier {
            {}, {},
            {}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
            vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
            gbufferAttachmentGroup.depthStencilAttachment->image, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil),
        });

        cb.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
            {},
            {}, {}, barriers);
    }
};