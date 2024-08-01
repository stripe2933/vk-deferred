export module vk_deferred:vulkan.buffer.SphereTransforms;

import std;
import glm;
export import vk_mem_alloc_hpp;
import vku;

namespace vk_deferred::vulkan::buffer {
    export struct SphereTransforms : vku::AllocatedBuffer {
        static constexpr std::uint32_t instanceCount = 25;

        explicit SphereTransforms(
            vma::Allocator allocator
        ) : AllocatedBuffer { vku::MappedBuffer { allocator, std::from_range, std::array {
                translate(glm::mat4 { 1.f }, { -5.f, 1.f, -5.f }),
                translate(glm::mat4 { 1.f }, { -5.f, 1.f, -2.5f }),
                translate(glm::mat4 { 1.f }, { -5.f, 1.f, 0.f }),
                translate(glm::mat4 { 1.f }, { -5.f, 1.f, 2.5f }),
                translate(glm::mat4 { 1.f }, { -5.f, 1.f, 5.f }),
                translate(glm::mat4 { 1.f }, { -2.5f, 1.f, -5.f }),
                translate(glm::mat4 { 1.f }, { -2.5f, 1.f, -2.5f }),
                translate(glm::mat4 { 1.f }, { -2.5f, 1.f, 0.f }),
                translate(glm::mat4 { 1.f }, { -2.5f, 1.f, 2.5f }),
                translate(glm::mat4 { 1.f }, { -2.5f, 1.f, 5.f }),
                translate(glm::mat4 { 1.f }, { 0.f, 1.f, -5.f }),
                translate(glm::mat4 { 1.f }, { 0.f, 1.f, -2.5f }),
                translate(glm::mat4 { 1.f }, { 0.f, 1.f, 0.f }),
                translate(glm::mat4 { 1.f }, { 0.f, 1.f, 2.5f }),
                translate(glm::mat4 { 1.f }, { 0.f, 1.f, 5.f }),
                translate(glm::mat4 { 1.f }, { 2.5f, 1.f, -5.f }),
                translate(glm::mat4 { 1.f }, { 2.5f, 1.f, -2.5f }),
                translate(glm::mat4 { 1.f }, { 2.5f, 1.f, 0.f }),
                translate(glm::mat4 { 1.f }, { 2.5f, 1.f, 2.5f }),
                translate(glm::mat4 { 1.f }, { 2.5f, 1.f, 5.f }),
                translate(glm::mat4 { 1.f }, { 5.f, 1.f, -5.f }),
                translate(glm::mat4 { 1.f }, { 5.f, 1.f, -2.5f }),
                translate(glm::mat4 { 1.f }, { 5.f, 1.f, 0.f }),
                translate(glm::mat4 { 1.f }, { 5.f, 1.f, 2.5f }),
                translate(glm::mat4 { 1.f }, { 5.f, 1.f, 5.f }),
            }, vk::BufferUsageFlagBits::eVertexBuffer }.unmap() } { }
    };
}