export module vk_deferred:Gpu;

import std;
import vku;
export import vulkan_hpp;

export class Gpu {
public:
    struct QueueFamilies {
        std::uint32_t graphicsPresent;

        QueueFamilies(
            const vk::raii::PhysicalDevice &physicalDevice [[clang::lifetimebound]],
            vk::SurfaceKHR surface
        ) {
            for (std::uint32_t i = 0; vk::QueueFamilyProperties properties : physicalDevice.getQueueFamilyProperties()) {
                if (properties.queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                    graphicsPresent = i;
                    return;
                }

                ++i;
            }

            throw std::runtime_error { "No suitable queue family found." };
        }
    };

    struct Queues {
        vk::Queue graphicsPresent;

        Queues(
            const vk::raii::Device &device,
            const QueueFamilies &queueFamilies
        ) noexcept : graphicsPresent { *device.getQueue(queueFamilies.graphicsPresent, 0) } { }
    };

    vk::raii::PhysicalDevice physicalDevice;
    QueueFamilies queueFamilies;
    vk::raii::Device device = createDevice();
    Queues queues { device, queueFamilies };
    vma::Allocator allocator;

    Gpu(
        const vk::raii::Instance &instance [[clang::lifetimebound]],
        vk::SurfaceKHR surface
    ) : physicalDevice { selectPhysicalDevice(instance, surface) },
        queueFamilies { physicalDevice, surface },
        allocator { createAllocator(*instance) } { }

    ~Gpu() {
        allocator.destroy();
    }

private:
    [[nodiscard]] auto selectPhysicalDevice(
        const vk::raii::Instance &instance,
        vk::SurfaceKHR surface
    ) const -> vk::raii::PhysicalDevice {
        auto adequatePhysicalDevices
            = instance.enumeratePhysicalDevices()
            | std::views::filter([&](const vk::raii::PhysicalDevice &physicalDevice) {
                try {
                    std::ignore = QueueFamilies { physicalDevice, surface };
                    return true;
                }
                catch (const std::runtime_error&) {
                    return false;
                }
            });
        if (adequatePhysicalDevices.empty()) {
            throw std::runtime_error { "No suitable GPU for the application" };
        }

        return *std::ranges::max_element(adequatePhysicalDevices, {}, [&](const vk::raii::PhysicalDevice &physicalDevice) {
            std::uint32_t score = 0;

            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                score += 1000;
            }

            score += properties.limits.maxImageDimension2D;

            return score;
        });
    }

    [[nodiscard]] auto createDevice() const -> vk::raii::Device {
        return { physicalDevice, vk::DeviceCreateInfo {
            {},
            vku::unsafeProxy({
                vk::DeviceQueueCreateInfo {
                    {},
                    0, // Assume first queue family supports both graphics and present operation.
                    vku::unsafeProxy({ 1.f }),
                },
            }),
            {},
            vku::unsafeProxy({
                vk::KHRSwapchainExtensionName,
                "VK_KHR_portability_subset",
            }),
        } };
    }

    [[nodiscard]] auto createAllocator(vk::Instance instance) const -> vma::Allocator {
        return vma::createAllocator(vma::AllocatorCreateInfo {
            {},
            *physicalDevice, *device,
            {}, {}, {}, {}, {},
            instance, vk::makeApiVersion(0, 1, 2, 0),
        });
    }
};