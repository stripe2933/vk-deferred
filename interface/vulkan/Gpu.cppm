module;

#include <version>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_deferred:vulkan.Gpu;

import std;
import vku;
export import vulkan_hpp;

namespace vk_deferred::vulkan {
    class QueueFamilies {
    public:
        std::uint32_t graphicsPresent;

        QueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
            : graphicsPresent { vku::getGraphicsPresentQueueFamily(physicalDevice, surface, physicalDevice.getQueueFamilyProperties()).value() } { }
    };

    struct Queues {
        vk::Queue graphicsPresent;

        Queues(vk::Device device, const QueueFamilies &queueFamilies) noexcept
            : graphicsPresent { device.getQueue(queueFamilies.graphicsPresent, 0) } { }

        [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept {
            return vku::RefHolder {
                [&](std::span<const float> queuePriorities) {
                    return std::array {
                        vk::DeviceQueueCreateInfo { {}, queueFamilies.graphicsPresent, queuePriorities },
                    };
                },
                std::array { 1.f },
            };
        }
    };

    export class Gpu : public vku::Gpu<QueueFamilies, Queues> {
    public:
        Gpu(const vk::raii::Instance &instance [[clang::lifetimebound]], vk::SurfaceKHR surface)
            : vku::Gpu<QueueFamilies, Queues> { instance, {
                .verbose = true,
                .deviceExtensions = {
#if __APPLE__
                    vk::KHRPortabilitySubsetExtensionName,
#endif
                    vk::KHRSwapchainExtensionName,
                },
                .queueFamilyGetter = [=](vk::PhysicalDevice physicalDevice) { return QueueFamilies { physicalDevice, surface }; },
                .apiVersion = vk::makeApiVersion(0, 1, 2, 0),
            } } { }
    };
}