#include <GLFW/glfw3.h>

#include <vulkan/vulkan_hpp_macros.hpp>

#ifdef _MSC_VER
import std;
#endif
import vulkan_hpp;
import vk_deferred;

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

    vk_deferred::MainApp{}.run();

    glfwTerminate();
}
