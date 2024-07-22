#include <GLFW/glfw3.h>

import vk_deferred;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    vk_deferred::MainApp{}.run();

    glfwTerminate();
}
