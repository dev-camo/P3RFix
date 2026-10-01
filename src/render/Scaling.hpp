#pragma once

#include <cstdint>
#include <optional>

namespace p3r::render
{
struct ScaleResult
{
    // The automatic baseline can exceed the float range for extreme inputs.
    // Keep it wide so diagnostics describe the value used before final clamping.
    double automaticMultiplier;
    float finalMultiplier;
    bool usedFallback;
};

// Automatic scaling preserves native quality below 1080 effective vertical
// pixels. An explicit user multiplier applies afterward, before final limits.
ScaleResult ComputeRenderTargetScale(int viewportHeight, float screenPercentage,
                                     float userMultiplier) noexcept;

struct ScaledDimensions
{
    std::int32_t width;
    std::int32_t height;
};

// Reject invalid or overflowing dimensions without producing a memory-write
// value. Valid positive products truncate toward zero, with a one-pixel minimum.
std::optional<ScaledDimensions> ScaleDimensions(std::int32_t width, std::int32_t height,
                                                float multiplier) noexcept;
} // namespace p3r::render
