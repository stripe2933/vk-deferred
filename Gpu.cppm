export module vk_deferred:Gpu;

import vku;
export import vulkan_hpp;

export class Gpu {
public:
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device = createDevice();
    vk::Queue queue = *device.getQueue(0, 0);
    vma::Allocator allocator;

    Gpu(
        const vk::raii::Instance &instance [[clang::lifetimebound]],
        vk::SurfaceKHR surface
    ) : physicalDevice { instance.enumeratePhysicalDevices().front() },
        allocator { createAllocator(*instance) } { }

    ~Gpu() {
        allocator.destroy();
    }

private:
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