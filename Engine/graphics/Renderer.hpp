#pragma once

#include <engine_export.h>

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <string>
#include <cstring>

enum class DrawMode {
    Triangles = 0,
    Line,
    Point,
};

struct RenderStats
{
    size_t unique_mesh = 0;
    size_t draw_call = 0;
    size_t vertices = 0;
    size_t indices = 0;

    auto reset() -> void { std::memset(this, 0, sizeof(*this)); }
};

class ENGINE_EXPORT IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual auto render(const class Scene& scene) const -> void = 0;

    virtual auto set_viewport(int32_t x, int32_t y, int32_t width, int32_t height) -> void = 0;
    virtual auto set_mode(DrawMode mode) -> void = 0;
    virtual auto clear_screen(uint32_t buffersmask) const -> void  = 0;
    virtual auto stats() const -> RenderStats = 0;
    virtual auto gpu_time_elapsed() const -> std::unordered_map<std::string, uint64_t>& = 0;
};
