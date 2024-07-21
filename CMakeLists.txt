cmake_minimum_required(VERSION 3.30)

# Enable CMake's experimental standard library module support.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(vk-deferred LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

# ----------------
# Include CPM.cmake.
# ----------------

file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/CPM.cmake)

# ----------------
# External dependencies.
# ----------------

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

CPMAddPackage(
    NAME vku
    GITHUB_REPOSITORY stripe2933/vku
    GIT_TAG module
    $<$<BOOL:${USE_VCPKG}>:"PATCHES ${CMAKE_CURRENT_BINARY_DIR}/_deps/vku-src/vcpkg-deps.patch"> # Apply patch if USE_VCPKG enabled.
    OPTIONS "VKU_USE_STD_MODULE ON"
)

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
        mod.cppm
        attachment_groups.cppm
        descriptor_set_layouts.cppm
        Frame.cppm
        Gpu.cppm
        MainApp.cppm
        meshes.cppm
        pipelines.cppm
        render_passes.cppm
        SharedData.cppm
)
target_link_libraries(vk-deferred PRIVATE
    glfw
    glm::module
    vku::vku
)
target_compile_definitions(vk-deferred PRIVATE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)