export module vk_deferred:vulkan.buffer.FloorTransforms;

import std;
import glm;
export import vk_mem_alloc_hpp;
import vku;

namespace vk_deferred::vulkan::buffer {
    export struct FloorTransforms : vku::MappedBuffer {
        static constexpr std::uint32_t instanceCount = 1;

        explicit FloorTransforms(
            vma::Allocator allocator
        ) : MappedBuffer { allocator, glm::mat4 { 1.f }, vk::BufferUsageFlagBits::eVertexBuffer } { }
    };
}