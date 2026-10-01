#pragma once

#include "render/Scaling.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>

// Include after the runner's Require(bool, const char*) declaration. Each case
// uses the actual shipped helper and independent expected arithmetic answers.
void RunRenderScalingTests(const std::function<void(const char*, const std::function<void()>&)>& runTest)
{
    using namespace p3r::render;

    runTest("automatic scale retains native baseline on a wide 720p viewport", [] {
        // Width is deliberately absent from the policy. A 2560x720 viewport
        // therefore has the same baseline as 1280x720; the old guard did not.
        const auto result = ComputeRenderTargetScale(720, 100.0f, 1.0f);
        Require(result.automaticMultiplier == 1.0, "720p automatic baseline");
        Require(result.finalMultiplier == 1.0f, "720p final multiplier");
        Require(!result.usedFallback, "valid 720p inputs need no fallback");
    });

    runTest("automatic scale retains native baseline at 800p and 1080p", [] {
        for (const int height : {800, 1080})
        {
            const auto result = ComputeRenderTargetScale(height, 100.0f, 1.0f);
            Require(result.automaticMultiplier == 1.0, "native automatic baseline");
            Require(result.finalMultiplier == 1.0f, "native final multiplier");
        }
    });

    runTest("automatic scale follows vertical rendering resolution", [] {
        const auto result1440 = ComputeRenderTargetScale(1440, 100.0f, 1.0f);
        Require(std::abs(result1440.automaticMultiplier - 1.3333333333333333) < 1e-12,
                "1440p automatic baseline");
        Require(std::abs(result1440.finalMultiplier - 1.3333333333333333f) < 1e-6f, "1440p final multiplier");

        // This also covers a narrow 1920x2160 viewport: width cannot suppress
        // the height-based baseline as it did in the old width-only guard.
        const auto result2160 = ComputeRenderTargetScale(2160, 100.0f, 1.0f);
        Require(result2160.automaticMultiplier == 2.0, "2160p automatic baseline");
        Require(result2160.finalMultiplier == 2.0f, "2160p final multiplier");
    });

    runTest("screen percentage changes the automatic baseline", [] {
        const auto half2160 = ComputeRenderTargetScale(2160, 50.0f, 1.0f);
        Require(half2160.automaticMultiplier == 1.0, "2160p at 50 percent baseline");
        Require(half2160.finalMultiplier == 1.0f, "2160p at 50 percent final");

        const auto boosted1440 = ComputeRenderTargetScale(1440, 150.0f, 1.0f);
        Require(boosted1440.automaticMultiplier == 2.0, "1440p at 150 percent baseline");
        Require(boosted1440.finalMultiplier == 2.0f, "1440p at 150 percent final");
    });

    runTest("explicit undersampling applies after the automatic baseline", [] {
        const auto result = ComputeRenderTargetScale(1440, 100.0f, 0.5f);
        Require(std::abs(result.automaticMultiplier - 1.3333333333333333) < 1e-12,
                "undersampling keeps the automatic baseline");
        Require(std::abs(result.finalMultiplier - 0.6666666666666667f) < 1e-6f,
                "explicit undersampling halves the automatic baseline");
    });

    runTest("only the combined multiplier receives upper and lower limits", [] {
        const auto highBaseline = ComputeRenderTargetScale(10800, 100.0f, 0.25f);
        Require(highBaseline.automaticMultiplier == 10.0, "automatic baseline is uncapped");
        Require(highBaseline.finalMultiplier == 2.5f, "undersampling precedes final cap");

        const auto capped = ComputeRenderTargetScale(2160, 100.0f, 4.0f);
        Require(capped.finalMultiplier == 4.0f, "combined upper limit");

        const auto floored = ComputeRenderTargetScale(1080, 100.0f, 0.125f);
        Require(floored.finalMultiplier == 0.25f, "combined lower limit");
        Require(!floored.usedFallback, "positive finite undersampling is valid");
    });

    runTest("invalid viewport heights use a temporary native baseline", [] {
        for (const int height : {0, -1, std::numeric_limits<int>::min()})
        {
            const auto result = ComputeRenderTargetScale(height, 150.0f, 0.5f);
            Require(result.automaticMultiplier == 1.0, "invalid height native baseline");
            Require(result.finalMultiplier == 0.5f, "invalid height still honors user multiplier");
            Require(result.usedFallback, "invalid height is reported");
        }
    });

    runTest("invalid screen percentages use 100 percent and report fallback", [] {
        const std::array invalidValues{0.0f, -100.0f, std::numeric_limits<float>::infinity(),
                                       -std::numeric_limits<float>::infinity(),
                                       std::numeric_limits<float>::quiet_NaN()};
        for (const float percentage : invalidValues)
        {
            const auto result = ComputeRenderTargetScale(2160, percentage, 0.5f);
            Require(result.automaticMultiplier == 2.0, "invalid percentage uses 100 percent");
            Require(result.finalMultiplier == 1.0f, "percentage fallback honors user multiplier");
            Require(result.usedFallback, "invalid percentage is reported");
        }
    });

    runTest("invalid user multipliers use one and report fallback", [] {
        const std::array invalidValues{0.0f, -1.0f, std::numeric_limits<float>::infinity(),
                                       -std::numeric_limits<float>::infinity(),
                                       std::numeric_limits<float>::quiet_NaN()};
        for (const float multiplier : invalidValues)
        {
            const auto result = ComputeRenderTargetScale(2160, 100.0f, multiplier);
            Require(result.automaticMultiplier == 2.0, "user fallback keeps automatic baseline");
            Require(result.finalMultiplier == 2.0f, "invalid user multiplier uses one");
            Require(result.usedFallback, "invalid user multiplier is reported");
        }
    });

    runTest("large finite inputs remain safe before the final clamp", [] {
        const auto result =
            ComputeRenderTargetScale(std::numeric_limits<int>::max(), std::numeric_limits<float>::max(),
                                     std::numeric_limits<float>::max());
        Require(std::isfinite(result.automaticMultiplier), "wide automatic arithmetic stays finite");
        Require(result.automaticMultiplier > std::numeric_limits<float>::max(),
                "automatic diagnostic retains values beyond the float range");
        Require(result.finalMultiplier == 4.0f, "large combined value receives final cap");
        Require(!result.usedFallback, "large positive finite inputs remain valid");
    });

    runTest("a baseline beyond the float range still permits combined undersampling", [] {
        const float maximum = std::numeric_limits<float>::max();
        const auto result = ComputeRenderTargetScale(216000, maximum, 1.0f / maximum);
        Require(result.automaticMultiplier > maximum, "automatic baseline exceeds float range");
        Require(std::abs(result.finalMultiplier - 2.0f) < 1e-6f,
                "wide automatic baseline combines before float narrowing");
        Require(!result.usedFallback, "finite extreme undersampling remains valid");
    });

    runTest("tiny positive inputs retain deliberate multiplier semantics", [] {
        const float tiny = std::numeric_limits<float>::denorm_min();
        const auto tinyPercentage = ComputeRenderTargetScale(2160, tiny, 1.0f);
        Require(tinyPercentage.automaticMultiplier == 1.0, "tiny percentage keeps native floor");
        Require(!tinyPercentage.usedFallback, "tiny positive percentage remains valid");

        const auto tinyMultiplier = ComputeRenderTargetScale(2160, 100.0f, tiny);
        Require(tinyMultiplier.finalMultiplier == 0.25f, "tiny positive user value reaches final floor");
        Require(!tinyMultiplier.usedFallback, "tiny positive user multiplier remains valid");
    });

    runTest("normal automatic scales produce exact expected target dimensions", [] {
        const auto scale1440 = ComputeRenderTargetScale(1440, 100.0f, 1.0f);
        const auto dimensions1440 = ScaleDimensions(1920, 1080, scale1440.finalMultiplier);
        Require(dimensions1440.has_value(), "1440p dimensions are valid");
        Require(dimensions1440->width == 2560, "1440p target width");
        Require(dimensions1440->height == 1440, "1440p target height");

        const auto scale2160 = ComputeRenderTargetScale(2160, 100.0f, 1.0f);
        const auto dimensions2160 = ScaleDimensions(1920, 1080, scale2160.finalMultiplier);
        Require(dimensions2160.has_value(), "2160p dimensions are valid");
        Require(dimensions2160->width == 3840, "2160p target width");
        Require(dimensions2160->height == 2160, "2160p target height");
    });

    runTest("positive scaled dimensions truncate toward zero", [] {
        const auto dimensions = ScaleDimensions(5, 7, 0.5f);
        Require(dimensions.has_value(), "fractional dimensions are valid");
        Require(dimensions->width == 2, "fractional width truncates");
        Require(dimensions->height == 3, "fractional height truncates");
    });

    runTest("valid undersampled dimensions retain at least one pixel", [] {
        const auto dimensions = ScaleDimensions(1, 3, 0.25f);
        Require(dimensions.has_value(), "small target is valid");
        Require(dimensions->width == 1, "undersampled width minimum");
        Require(dimensions->height == 1, "undersampled height minimum");

        const auto tiny = ScaleDimensions(1, 1, std::numeric_limits<float>::denorm_min());
        Require(tiny.has_value(), "tiny positive scale is valid");
        Require(tiny->width == 1 && tiny->height == 1, "tiny scale preserves one pixel");
    });

    runTest("nonpositive source dimensions are rejected", [] {
        Require(!ScaleDimensions(0, 1080, 1.0f), "zero width is rejected");
        Require(!ScaleDimensions(1920, 0, 1.0f), "zero height is rejected");
        Require(!ScaleDimensions(-1, 1080, 1.0f), "negative width is rejected");
        Require(!ScaleDimensions(1920, -1, 1.0f), "negative height is rejected");
    });

    runTest("invalid dimension multipliers are rejected", [] {
        const std::array invalidValues{0.0f, -1.0f, std::numeric_limits<float>::infinity(),
                                       -std::numeric_limits<float>::infinity(),
                                       std::numeric_limits<float>::quiet_NaN()};
        for (const float multiplier : invalidValues)
            Require(!ScaleDimensions(1920, 1080, multiplier), "invalid dimension scale");
    });

    runTest("the largest int32 dimension is accepted without rounding upward", [] {
        constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
        const auto dimensions = ScaleDimensions(maximum, maximum, 1.0f);
        Require(dimensions.has_value(), "largest int32 dimensions remain valid");
        Require(dimensions->width == maximum && dimensions->height == maximum,
                "double arithmetic preserves exact int32 dimensions");
    });

    runTest("integer overflow is rejected before dimensions are narrowed", [] {
        constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
        Require(!ScaleDimensions(maximum, 1, 2.0f), "width overflow is rejected");
        Require(!ScaleDimensions(1, maximum, 2.0f), "height overflow is rejected");
        Require(!ScaleDimensions(maximum, 1, std::nextafter(1.0f, 2.0f)),
                "scale just above one would overflow width");
        Require(!ScaleDimensions(1, 1, std::numeric_limits<float>::max()),
                "large finite product is rejected");
    });

    runTest("integer upper-bound checks distinguish adjacent source dimensions", [] {
        const auto valid = ScaleDimensions(1073741823, 1, 2.0f);
        Require(valid.has_value(), "product immediately below int32 limit is valid");
        Require(valid->width == 2147483646, "valid near-limit width is exact");
        Require(!ScaleDimensions(1073741824, 1, 2.0f), "product immediately above int32 limit is rejected");
    });
}
