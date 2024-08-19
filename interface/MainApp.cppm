module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_deferred:MainApp;

import std;
import vku;
import :vulkan.Frame;
import :vulkan.buffer.LightInstances;

#define INDEX_SEQ(Is, N, ...) [&]<std::size_t ...Is>(std::index_sequence<Is...>) __VA_ARGS__ (std::make_index_sequence<N>{})
#define ARRAY_OF(N, ...) INDEX_SEQ(Is, N, { return std::array { ((void)Is, __VA_ARGS__)... }; })

namespace vk_deferred {
    export class MainApp {
    public:
        ~MainApp() {
            glfwDestroyWindow(window);
        }

        auto run() const -> void {
            const vulkan::SharedData sharedData { gpu, *surface };
            const std::uint32_t seed = std::random_device{}();
            std::array frames = ARRAY_OF(2, vulkan::Frame { gpu, sharedData, seed });

            for (std::uint64_t frameIndex = 0; !glfwWindowShouldClose(window); ++frameIndex){
                glfwPollEvents();
                frames[(frameIndex % frames.size())].onLoop(glfwGetTime());
            }

            gpu.device.waitIdle();
        }

    private:
        GLFWwindow *window = glfwCreateWindow(640, 480, std::format("Vulkan Deferred Rendering ({} Lights)", vulkan::buffer::LightInstances::instanceCount).c_str(), nullptr, nullptr);

        vk::raii::Context context;
        vk::raii::Instance instance = createInstance();
        vk::raii::SurfaceKHR surface = createSurface();
        vulkan::Gpu gpu { instance, *surface };

        [[nodiscard]] auto createInstance() const -> vk::raii::Instance {
            std::vector<const char*> extensions {
#if __APPLE__
                vk::KHRPortabilityEnumerationExtensionName,
#endif
            };

            // Add Vulkan extensions for GLFW.
            std::uint32_t glfwExtensionCount;
            const auto* glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            extensions.append_range(std::views::counted(glfwExtensions, glfwExtensionCount));

            vk::raii::Instance instance { context, vk::InstanceCreateInfo {
#if __APPLE__
                vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
#else
                {},
#endif
                vku::unsafeAddress(vk::ApplicationInfo {
                    "Vulkan Deferred Rendering", 0,
                    {}, 0,
                    vk::makeApiVersion(0, 1, 2, 0), // MoltenVK conforms Vulkan 1.2.
                }),
                {},
                extensions,
            } };

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
#endif
            return instance;
        }

        [[nodiscard]] auto createSurface() const -> vk::raii::SurfaceKHR {
            if (VkSurfaceKHR surface; glfwCreateWindowSurface(*instance, window, nullptr, &surface) == VK_SUCCESS) {
                return { instance, surface };
            }

            const char *errorMessage;
            const int errorCode = glfwGetError(&errorMessage);
            throw std::runtime_error { std::format("Failed to create GLFW window surface: {} (error code={})", errorMessage, errorCode) };
        }
    };
}