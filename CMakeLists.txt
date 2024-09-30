cmake_minimum_required(VERSION 3.30)

project(vk-deferred LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

# ----------------
# External dependencies.
# ----------------

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(vku CONFIG REQUIRED)

# ----------------
# Module configurations for external dependencies.
# ----------------

if (NOT TARGET glm_module)
    add_library(glm_module)
    target_sources(glm_module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS extlibs
        FILES extlibs/module-ports/glm.cppm
    )
    target_link_libraries(glm_module PUBLIC glm::glm)

    target_compile_definitions(glm_module PUBLIC
        GLM_GTC_INLINE_NAMESPACE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_XYZW_ONLY
    )

    add_library(glm::module ALIAS glm_module)
endif()

# ----------------
# Project targets.
# ----------------

add_executable(vk-deferred main.cpp)
target_sources(vk-deferred PRIVATE
    FILE_SET CXX_MODULES
    FILES
        interface/mod.cppm
        interface/MainApp.cppm
        interface/vulkan/attachment_group/DeferredLighting.cppm
        interface/vulkan/attachment_group/GBuffer.cppm
        interface/vulkan/attachment_group/Swapchain.cppm
        interface/vulkan/descriptor_set_layout/GBufferInput.cppm
        interface/vulkan/descriptor_set_layout/HdrInput.cppm
        interface/vulkan/Frame.cppm
        interface/vulkan/Gpu.cppm
        interface/vulkan/mesh/Floor.cppm
        interface/vulkan/mesh/Sphere.cppm
        interface/vulkan/buffer/FloorTransforms.cppm
        interface/vulkan/buffer/LightInstances.cppm
        interface/vulkan/buffer/SphereTransforms.cppm
        interface/vulkan/pipeline/DeferredLightRenderer.cppm
        interface/vulkan/pipeline/GBufferRenderer.cppm
        interface/vulkan/pipeline/ToneMappingRenderer.cppm
        interface/vulkan/render_pass/Deferred.cppm
        interface/vulkan/SharedData.cppm
)
target_link_libraries(vk-deferred PRIVATE
    glfw
    glm::module
    vku::vku
)
target_compile_definitions(vk-deferred PRIVATE
    GLFW_INCLUDE_NONE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)