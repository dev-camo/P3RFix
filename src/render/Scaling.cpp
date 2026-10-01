#include "Scaling.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace p3r::render
{
ScaleResult ComputeRenderTargetScale(int viewportHeight, float screenPercentage,
                                     float userMultiplier) noexcept
{
    bool usedFallback = viewportHeight <= 0;

    if (!std::isfinite(screenPercentage) || screenPercentage <= 0.0f)
    {
        screenPercentage = 100.0f;
        usedFallback = true;
    }

    if (!std::isfinite(userMultiplier) || userMultiplier <= 0.0f)
    {
        userMultiplier = 1.0f;
        usedFallback = true;
    }

    double automaticMultiplier = 1.0;
    if (viewportHeight > 0)
    {
        const double effectiveHeight = static_cast<double>(viewportHeight) * screenPercentage / 100.0;
        automaticMultiplier = std::max(1.0, effectiveHeight / 1080.0);
    }

    // Apply limits to the combined value, not the automatic baseline. A high
    // baseline and deliberate undersampling must keep their relative meaning.
    const double combinedMultiplier = automaticMultiplier * userMultiplier;
    const float finalMultiplier = static_cast<float>(std::clamp(combinedMultiplier, 0.25, 4.0));

    return {automaticMultiplier, finalMultiplier, usedFallback};
}

std::optional<ScaledDimensions> ScaleDimensions(std::int32_t width, std::int32_t height,
                                                float multiplier) noexcept
{
    if (width <= 0 || height <= 0 || !std::isfinite(multiplier) || multiplier <= 0.0f)
    {
        return std::nullopt;
    }

    const double scaledWidth = static_cast<double>(width) * multiplier;
    const double scaledHeight = static_cast<double>(height) * multiplier;
    constexpr double maximumDimension = static_cast<double>(std::numeric_limits<std::int32_t>::max());

    // Check floating-point bounds before either cast: casting an out-of-range
    // value to an integer is unsafe even when a later clamp would discard it.
    if (!std::isfinite(scaledWidth) || !std::isfinite(scaledHeight) || scaledWidth > maximumDimension ||
        scaledHeight > maximumDimension)
    {
        return std::nullopt;
    }

    return ScaledDimensions{static_cast<std::int32_t>(std::max(1.0, scaledWidth)),
                            static_cast<std::int32_t>(std::max(1.0, scaledHeight))};
}
} // namespace p3r::render
