module;

#include <version>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_deferred:vulkan.Gpu;

import std;
import vku;
export import vulkan_hpp;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define CHECK_FEATURE(feature) if (physicalDeviceFeatures->feature && !availableFeatures.feature) { unavailableFeatures.push_back(#feature); }

template <typename T, typename... Ts>
concept one_of = (std::same_as<T, Ts> || ...);

template <typename R, typename T>
concept contiguous_range_of = std::ranges::contiguous_range<R> && std::same_as<std::ranges::range_value_t<R>, T>;

namespace vku {
    /**
     * Get compute capable queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    [[nodiscard]] auto getComputeQueueFamily(
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get compute specialized (queue flag without graphics) queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the compute specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    [[nodiscard]] auto getComputeSpecializedQueueFamily(
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute && !(properties.queueFlags & vk::QueueFlagBits::eGraphics)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get graphics capable queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the graphics capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    [[nodiscard]] auto getGraphicsQueueFamily(
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get transfer specialized (queue flag without both compute and graphics) queue family index.
     * @param queueFamilyProperties Queue family properties. Could be enumerated by <tt>vk::PhysicalDevice::getQueueFamilyProperties</tt>.
     * @return The index of the transfer specialized queue family, or <tt>std::nullopt</tt> if not found.
     */
    [[nodiscard]] auto getTransferQueueFamily(
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) noexcept -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eTransfer && !(properties.queueFlags & (vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eGraphics))) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyCount Number of queue families, which is identical to <tt>physicalDevice.getQueueFamilyProperties().size()</tt>.
     * @return The index of the present capable queue family, or <tt>std::nullopt</tt> if not found.
     */
    [[nodiscard]] auto getPresentQueueFamily(
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface,
        std::uint32_t queueFamilyCount
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
        }
        return std::nullopt;
    }

    /**
     * Get both graphics and present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both graphics and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note
     * This is useful for the common case of rendering to a window, because it doesn't requires the explicit queue family
     * ownership transfer between graphics and present queues.
     */
    [[nodiscard]] auto getGraphicsPresentQueueFamily(
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface,
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    /**
     * Get both compute and present capable queue family index.
     * @param physicalDevice Vulkan physical device to get queue families from.
     * @param surface Surface to test for presentation support.
     * @param queueFamilyProperties Queue family properties, which is identical to <tt>physicalDevice.getQueueFamilyProperties()</tt>.
     * @return The index of the both compute and present capable queue family, or <tt>std::nullopt</tt> if not found.
     * @note
     * This is useful for compute based window rendering, because it doesn't requires the explicit queue family ownership
     * transfer between compute and present queues.
     */
    [[nodiscard]] auto getComputePresentQueueFamily(
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface,
        std::span<const vk::QueueFamilyProperties> queueFamilyProperties
    ) -> std::optional<std::uint32_t> {
        for (std::uint32_t i = 0; const vk::QueueFamilyProperties &properties : queueFamilyProperties) {
            if (properties.queueFlags & vk::QueueFlagBits::eCompute && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                return i;
            }
            ++i;
        }
        return std::nullopt;
    }

    template <typename QueueFamilies, std::constructible_from<vk::Device, const QueueFamilies&> Queues> requires
        requires(vk::PhysicalDevice physicalDevice, const QueueFamilies &queueFamilies) {
            { Queues::getCreateInfos(physicalDevice, queueFamilies).get() } -> contiguous_range_of<vk::DeviceQueueCreateInfo>;
        }
    class Gpu {
        [[nodiscard]] static auto getQueueFamilies(vk::PhysicalDevice physicalDevice) noexcept -> QueueFamilies {
            return QueueFamilies { physicalDevice };
        }

        [[nodiscard]] static auto ratePhysicalDevice(
            vk::PhysicalDevice physicalDevice,
            bool verbose,
            const std::function<QueueFamilies(vk::PhysicalDevice)> &queueFamilyGetter,
            std::span<const char* const> deviceExtensions,
            const vk::PhysicalDeviceFeatures *physicalDeviceFeatures = nullptr
        ) -> std::uint32_t {
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            const std::string_view deviceName { properties.deviceName.data() };

            // Check queue family availability.
            try {
                std::ignore = queueFamilyGetter(physicalDevice);
            }
            catch (const std::runtime_error &e) {
                if (verbose) {
                    std::println(std::cerr, "Physical device \"{}\" rejected because it failed to get the request queue families: {}", deviceName, e.what());
                }
                return 0;
            }

            // Check device extension availability.
            constexpr auto toCStr = [](const vk::ExtensionProperties& properties) { return properties.extensionName.data(); };
            std::vector availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            std::ranges::sort(availableExtensions, std::strcmp, toCStr);
            if (!std::ranges::includes(availableExtensions, deviceExtensions, std::strcmp, toCStr)) {
                if (verbose) {
                    std::vector<const char*> unavailableExtensions;
                    std::ranges::set_difference(deviceExtensions, availableExtensions | std::views::transform(toCStr), std::back_inserter(unavailableExtensions), std::strcmp);
#if __cpp_lib_format_ranges >= 202207L
                    std::println(std::cerr, "Physical device \"{}\" rejected because it lacks the following device extensions: {::s}", deviceName, unavailableExtensions);
#else
                    std::print(std::cerr, "Physical device \"{}\" rejected because it lacks the following device extensions: [", deviceName);
                    for (std::size_t i = 0; i < unavailableExtensions.size(); ++i) {
                        if (i == unavailableExtensions.size() - 1) {
                            std::println(std::cerr, "{}]", unavailableExtensions[i]);
                }
                        else {
                            std::print(std::cerr, "{}, ", unavailableExtensions[i]);
                        }
                    }
#endif
                }
                return 0;
            }

            // Check physical device feature availability.
            const vk::PhysicalDeviceFeatures availableFeatures = physicalDevice.getFeatures();
            if (physicalDeviceFeatures) {
                // I hope vk::PhysicalDeviceFeatures struct does not change in the future...
                std::vector<const char*> unavailableFeatures;
                CHECK_FEATURE(robustBufferAccess);
                CHECK_FEATURE(fullDrawIndexUint32);
                CHECK_FEATURE(imageCubeArray);
                CHECK_FEATURE(independentBlend);
                CHECK_FEATURE(geometryShader);
                CHECK_FEATURE(tessellationShader);
                CHECK_FEATURE(sampleRateShading);
                CHECK_FEATURE(dualSrcBlend);
                CHECK_FEATURE(logicOp);
                CHECK_FEATURE(multiDrawIndirect);
                CHECK_FEATURE(drawIndirectFirstInstance);
                CHECK_FEATURE(depthClamp);
                CHECK_FEATURE(depthBiasClamp);
                CHECK_FEATURE(fillModeNonSolid);
                CHECK_FEATURE(depthBounds);
                CHECK_FEATURE(wideLines);
                CHECK_FEATURE(largePoints);
                CHECK_FEATURE(alphaToOne);
                CHECK_FEATURE(multiViewport);
                CHECK_FEATURE(samplerAnisotropy);
                CHECK_FEATURE(textureCompressionETC2);
                CHECK_FEATURE(textureCompressionASTC_LDR);
                CHECK_FEATURE(textureCompressionBC);
                CHECK_FEATURE(occlusionQueryPrecise);
                CHECK_FEATURE(pipelineStatisticsQuery);
                CHECK_FEATURE(vertexPipelineStoresAndAtomics);
                CHECK_FEATURE(fragmentStoresAndAtomics);
                CHECK_FEATURE(shaderTessellationAndGeometryPointSize);
                CHECK_FEATURE(shaderImageGatherExtended);
                CHECK_FEATURE(shaderStorageImageExtendedFormats);
                CHECK_FEATURE(shaderStorageImageMultisample);
                CHECK_FEATURE(shaderStorageImageReadWithoutFormat);
                CHECK_FEATURE(shaderStorageImageWriteWithoutFormat);
                CHECK_FEATURE(shaderUniformBufferArrayDynamicIndexing);
                CHECK_FEATURE(shaderSampledImageArrayDynamicIndexing);
                CHECK_FEATURE(shaderStorageBufferArrayDynamicIndexing);
                CHECK_FEATURE(shaderStorageImageArrayDynamicIndexing);
                CHECK_FEATURE(shaderClipDistance);
                CHECK_FEATURE(shaderCullDistance);
                CHECK_FEATURE(shaderFloat64);
                CHECK_FEATURE(shaderInt64);
                CHECK_FEATURE(shaderInt16);
                CHECK_FEATURE(shaderResourceResidency);
                CHECK_FEATURE(shaderResourceMinLod);
                CHECK_FEATURE(sparseBinding);
                CHECK_FEATURE(sparseResidencyBuffer);
                CHECK_FEATURE(sparseResidencyImage2D);
                CHECK_FEATURE(sparseResidencyImage3D);
                CHECK_FEATURE(sparseResidency2Samples);
                CHECK_FEATURE(sparseResidency4Samples);
                CHECK_FEATURE(sparseResidency8Samples);
                CHECK_FEATURE(sparseResidency16Samples);
                CHECK_FEATURE(sparseResidencyAliased);
                CHECK_FEATURE(variableMultisampleRate);
                CHECK_FEATURE(inheritedQueries);

                if (!unavailableFeatures.empty()) {
                    if (verbose) {
#if __cpp_lib_format_ranges >= 202207L
                        std::println(std::cerr, "Physical device \"{}\" rejected because it lacks the following physical device features: {::s}", deviceName, unavailableFeatures);
#else
                        std::print(std::cerr, "Physical device \"{}\" rejected because it lacks the following physical device features: [", deviceName);
                        for (std::size_t i = 0; i < unavailableFeatures.size(); ++i) {
                            if (i == unavailableFeatures.size() - 1) {
                                std::println(std::cerr, "{}]", unavailableFeatures[i]);
                    }
                            else {
                                std::print(std::cerr, "{}, ", unavailableFeatures[i]);
                            }
                        }
#endif
                    }
                    return 0;
                }
            }

            std::uint32_t score = 0;
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                score += 1000;
            }

            score += properties.limits.maxImageDimension2D;

            if (verbose) {
                std::println(std::cerr, "Physical device \"{}\" accepted (score={}).", deviceName, score);
            }
            return score;
        }

    public:
        template <typename... DevicePNexts>
        struct Config {
            static constexpr bool hasPhysicalDeviceFeatures = !one_of<vk::PhysicalDeviceFeatures2, DevicePNexts...>;

            bool verbose = false;
            std::vector<const char*> deviceExtensions = {};
            [[no_unique_address]]
            std::conditional_t<hasPhysicalDeviceFeatures, vk::PhysicalDeviceFeatures, std::monostate> physicalDeviceFeatures = {};
            std::function<QueueFamilies(vk::PhysicalDevice)> queueFamilyGetter = &getQueueFamilies;
            std::function<std::uint32_t(vk::PhysicalDevice)> physicalDeviceRater
                // TODO.CXX23: use std::bind_back for readability.
                = [this](vk::PhysicalDevice physicalDevice) {
                    if constexpr (hasPhysicalDeviceFeatures) {
                        return ratePhysicalDevice(physicalDevice, verbose, queueFamilyGetter, deviceExtensions, &physicalDeviceFeatures);
                    }
                    else {
                        return ratePhysicalDevice(physicalDevice, verbose, queueFamilyGetter, deviceExtensions);
                    }
                };
            std::tuple<DevicePNexts...> devicePNexts = {};
            vma::AllocatorCreateFlags allocatorCreateFlags = {};
            std::uint32_t apiVersion = vk::makeApiVersion(0, 1, 0, 0);
        };

        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilies queueFamilies;
        vk::raii::Device device;
        Queues queues { *device, queueFamilies };
        vma::Allocator allocator;

        template <typename... DevicePNexts>
        explicit Gpu(
            const vk::raii::Instance &instance [[clang::lifetimebound]],
            Config<DevicePNexts...> config = {}
        ) : physicalDevice { selectPhysicalDevice(instance, config) },
            queueFamilies { config.queueFamilyGetter(physicalDevice) },
            device { createDevice(config) },
            allocator { createAllocator(instance, config) } { }

        ~Gpu() {
            allocator.destroy();
        }

    private:
        template <typename... DevicePNexts>
        [[nodiscard]] auto selectPhysicalDevice(
            const vk::raii::Instance &instance,
            Config<DevicePNexts...> &config
        ) const -> vk::raii::PhysicalDevice {
            std::ranges::sort(config.deviceExtensions);

            auto adequatePhysicalDevices
                = instance.enumeratePhysicalDevices()
                | std::views::filter([&](vk::PhysicalDevice physicalDevice) {
                    return config.physicalDeviceRater(physicalDevice) > 0;
                });
            if (adequatePhysicalDevices.empty()) {
                throw std::runtime_error { "No suitable GPU for the application" };
            }

            return *std::ranges::max_element(adequatePhysicalDevices, {}, config.physicalDeviceRater);
        }

        template <typename... PNexts>
        [[nodiscard]] auto createDevice(
            const Config<PNexts...> &config
        ) const -> vk::raii::Device {
            // This have to be here, because after end of the RefHolder::get() expression, queue priorities are
            // destroyed (which makes the pointer to them becomes invalid), but it is still in the std::apply scope.
            const auto queueCreateInfos = Queues::getCreateInfos(*physicalDevice, queueFamilies);
            vk::raii::Device device { physicalDevice, std::apply([&](const auto &...pNexts) {
                const vk::PhysicalDeviceFeatures *pPhysicalDeviceFeatures = nullptr;
                if constexpr (Config<PNexts...>::hasPhysicalDeviceFeatures) {
                    pPhysicalDeviceFeatures = &config.physicalDeviceFeatures;
                }

                /* Note:
                 * Directly returning vk::StructureChain will cause runtime error, because pNexts pointer chain gets
                 * invalidated. Creating non-const result value and returning it works because of the RVO. After C++17,
                 * RVO is guaranteed by the standard. */
                vk::StructureChain createInfo {
                    vk::DeviceCreateInfo {
                        {},
                        queueCreateInfos.get(),
                        {},
                        config.deviceExtensions,
                        pPhysicalDeviceFeatures,
                    },
                    pNexts...,
                };

                return createInfo;
            }, config.devicePNexts).get() };

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
#endif
            return device;
        }

        template <typename... DevicePNexts>
        [[nodiscard]] auto createAllocator(
            const vk::raii::Instance &instance,
            const Config<DevicePNexts...> &config
        ) const -> vma::Allocator {
            return vma::createAllocator(vma::AllocatorCreateInfo {
                config.allocatorCreateFlags,
                *physicalDevice, *device,
                {}, {}, {}, {},
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
                vku::unsafeAddress(vma::VulkanFunctions{
                    instance.getDispatcher()->vkGetInstanceProcAddr,
                    device.getDispatcher()->vkGetDeviceProcAddr,
                }),
#else
                {},
#endif
                *instance, config.apiVersion,
            });
        }
    };
}

namespace vk_deferred::vulkan {
    class QueueFamilies {
    public:
        std::uint32_t graphicsPresent;

        QueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
            : QueueFamilies { physicalDevice, surface, physicalDevice.getQueueFamilyProperties() } { }

    private:
        QueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, std::span<const vk::QueueFamilyProperties> queueFamilyProperties)
            : graphicsPresent { vku::getGraphicsPresentQueueFamily(physicalDevice, surface, queueFamilyProperties).value() } { }
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