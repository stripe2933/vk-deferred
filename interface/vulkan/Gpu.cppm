module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_deferred:vulkan.Gpu;

import std;
import vku;
export import vulkan_hpp;

namespace vk_deferred::vulkan {
    struct QueueFamilies {
        std::uint32_t graphicsPresent;

        QueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
            : graphicsPresent { vku::getGraphicsPresentQueueFamily(physicalDevice, surface, physicalDevice.getQueueFamilyProperties()).value() } { }
    };

    struct Queues {
        vk::Queue graphicsPresent;

        Queues(vk::Device device, const QueueFamilies &queueFamilies) noexcept
            : graphicsPresent { device.getQueue(queueFamilies.graphicsPresent, 0) } { }

        [[nodiscard]] static auto getCreateInfos(vk::PhysicalDevice, const QueueFamilies &queueFamilies) noexcept
#ifdef _MSC_VER
            -> vku::RefHolder<std::array<vk::DeviceQueueCreateInfo, 1>, std::array<float, 1>>
#endif
        {
            return vku::RefHolder {
                [&](std::span<const float> priorities) {
                    return std::array {
                        vk::DeviceQueueCreateInfo {
                            {},
                            queueFamilies.graphicsPresent,
                            priorities,
                        },
                    };
                },
                std::array { 1.f },
            };
        }
    };

    export struct Gpu : vku::Gpu<QueueFamilies, Queues> {
        Gpu(const vk::raii::Instance &instance [[clang::lifetimebound]], vk::SurfaceKHR surface)
            : vku::Gpu<QueueFamilies, Queues> { instance, Config<> {
                .verbose = true,
                .deviceExtensions = {
#if __APPLE__
                    vk::KHRPortabilitySubsetExtensionName,
#endif
                    vk::KHRSwapchainExtensionName,
                },
                .queueFamilyGetter = [=](vk::PhysicalDevice physicalDevice) -> QueueFamilies {
                    return { physicalDevice, surface };
                },
                .apiVersion = vk::makeApiVersion(0, 1, 2, 0),
            } } { }
    };
}