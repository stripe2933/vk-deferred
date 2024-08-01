export module vk_deferred:vulkan.buffer.LightInstances;

import std;
import glm;
export import vk_mem_alloc_hpp;
import vku;

namespace vk_deferred::vulkan::buffer {
    export class LightInstances : public vku::MappedBuffer {
    public:
        struct InstanceType {
            glm::vec3 center;
            float radius;
            glm::vec3 color;
        };

        static constexpr std::uint32_t instanceCount = 1500;

        explicit LightInstances(
            vma::Allocator allocator,
            std::mt19937 randomGenerator
        ) : MappedBuffer { allocator, std::from_range, std::views::iota(0U, instanceCount) | std::views::transform([&](std::uint32_t) {
            std::uniform_real_distribution radiusDist { 0.5f, 2.f };
            std::uniform_real_distribution hueDist { 0.f, 1.f };
            return InstanceType { .radius = radiusDist(randomGenerator), .color = hsvToRgb(hueDist(randomGenerator), 0.8f, 0.8f) };
        }), vk::BufferUsageFlagBits::eVertexBuffer } {
            std::uniform_real_distribution piDist { 0.f, 2.f * std::numbers::pi_v<float> };
            std::generate_n(back_inserter(oscillations), instanceCount, [&] {
                return std::array {
                    Oscillation { 10.f, 0.1f * piDist(randomGenerator), piDist(randomGenerator) },
                    Oscillation { 1.f, 0.1f * piDist(randomGenerator), piDist(randomGenerator) },
                    Oscillation { 10.f, 0.1f * piDist(randomGenerator), piDist(randomGenerator) },
                };
            });
        }

        auto update(float time) -> void {
            for (auto &&[center, oscillation3d] : std::views::zip(asRange<InstanceType>() | std::views::transform(&InstanceType::center), oscillations)) {
                center = glm::vec3 {
                    oscillation3d[0].displacement(time),
                    oscillation3d[1].displacement(time),
                    oscillation3d[2].displacement(time),
                } + glm::vec3 { 0.f, 1.5f, 0.f };
            }
        }

    private:
        struct Oscillation {
            float amplitude;
            float angularVelocity;
            float phase;

            [[nodiscard]] auto displacement(float t) const -> float {
                return amplitude * std::sin(angularVelocity * t + phase);
            }
        };

        std::vector<std::array<Oscillation, 3>> oscillations;

        [[nodiscard]] static auto hsvToRgb(float h, float s, float v) noexcept -> glm::vec3 {
            const int i = static_cast<int>(h * 6);
            const float f = h * 6 - i;
            const float p = v * (1 - s);
            const float q = v * (1 - f * s);
            const float t = v * (1 - (1 - f) * s);

            switch (i % 6) {
                case 0: return { v, t, p };
                case 1: return { q, v, p };
                case 2: return { p, v, t };
                case 3: return { p, q, v };
                case 4: return { t, p, v };
                case 5: return { v, p, q };
            }
            std::unreachable();
        }
    };
}