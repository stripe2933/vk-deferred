module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module vk_deferred:MainApp;

import std;
import vku;
import :Frame;
import :SharedData;

export class MainApp {
public:
    ~MainApp() {
        glfwDestroyWindow(window);
    }

    auto run() const -> void {
        const SharedData sharedData { gpu, *surface };
        Frame frame { gpu, sharedData };

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            frame.onLoop(glfwGetTime());
        }

        gpu.device.waitIdle();
    }

private:
    GLFWwindow *window = glfwCreateWindow(640, 480, "Vulkan Deferred Rendering", nullptr, nullptr);

    vk::raii::Context context;
    vk::raii::Instance instance = createInstance();
    vk::raii::SurfaceKHR surface = createSurface();
    Gpu gpu { instance, *surface };

    [[nodiscard]] auto createInstance() const -> vk::raii::Instance {
        std::vector extensions {
            vk::KHRPortabilityEnumerationExtensionName,
        };

        // Add Vulkan extensions for GLFW.
        std::uint32_t glfwExtensionCount;
        extensions.append_range(std::views::counted(glfwGetRequiredInstanceExtensions(&glfwExtensionCount), glfwExtensionCount));

        return { context, vk::InstanceCreateInfo {
            vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
            vku::unsafeAddress(vk::ApplicationInfo {
                "Vulkan Deferred Rendering", 0,
                {}, 0,
                vk::makeApiVersion(0, 1, 2, 0), // MoltenVK conforms Vulkan 1.2.
            }),
#ifdef NDEBUG
            {},
#else
            vku::unsafeProxy({
                "VK_LAYER_KHRONOS_validation",
            }),
#endif
            extensions,
        } };
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