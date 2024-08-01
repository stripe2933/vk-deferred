export module vk_deferred:vulkan.mesh.Floor;

import std;
import glm;
export import vk_mem_alloc_hpp;
import vku;

namespace vk_deferred::vulkan::mesh {
    export struct Floor {
        struct VertexType {
            glm::vec3 position;
            glm::vec3 normal;
        };

        static constexpr std::uint32_t drawCount = 6;

        vku::AllocatedBuffer vertexBuffer;

        explicit Floor(
            vma::Allocator allocator
        ) : vertexBuffer { vku::MappedBuffer { allocator, std::from_range, std::array<VertexType, drawCount> {
                VertexType { { -25.f, 0.f, -25.f }, { 0.f, 1.f, 0.f } },
                VertexType { { -25.f, 0.f,  25.f }, { 0.f, 1.f, 0.f } },
                VertexType { {  25.f, 0.f,  25.f }, { 0.f, 1.f, 0.f } },
                VertexType { { -25.f, 0.f, -25.f }, { 0.f, 1.f, 0.f } },
                VertexType { {  25.f, 0.f,  25.f }, { 0.f, 1.f, 0.f } },
                VertexType { {  25.f, 0.f, -25.f }, { 0.f, 1.f, 0.f } },
            }, vk::BufferUsageFlagBits::eVertexBuffer }.unmap() } { }
    };
}