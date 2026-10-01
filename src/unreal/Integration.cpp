#include "Integration.hpp"

#include "detail/Layouts.hpp"
#include "detail/Runtime.hpp"

namespace p3r::unreal {

InitStatus InitializeConsoleRuntime(RuntimeAddresses addresses) noexcept {
    return detail::InitializeRuntime(addresses);
}

std::optional<RenderTargetInfo> ReadRenderTarget(const void* object) noexcept {
    if (!object) return std::nullopt;
    return RenderTargetInfo{
        detail::ReadMemory<std::int32_t>(object, offsetof(detail::RenderTargetView, sizeX)),
        detail::ReadMemory<std::int32_t>(object, offsetof(detail::RenderTargetView, sizeY)),
        detail::ReadMemory<std::uint8_t>(object, offsetof(detail::RenderTargetView, renderTargetFormat)) == detail::Rgba16fFormat
    };
}

bool SetRenderTargetSize(void* object, std::int32_t width, std::int32_t height) noexcept {
    if (!object) return false;
    detail::WriteMemory(object, offsetof(detail::RenderTargetView, sizeX), width);
    detail::WriteMemory(object, offsetof(detail::RenderTargetView, sizeY), height);
    return true;
}

bool SetCaptureSize(void* object, std::int32_t width, std::int32_t height) noexcept {
    if (!object) return false;
    detail::WriteMemory(object, offsetof(detail::CaptureView, width), width);
    detail::WriteMemory(object, offsetof(detail::CaptureView, height), height);
    return true;
}

ConsoleResult TryEnableConsole() {
    return detail::TryEnableConsole();
}

} // namespace p3r::unreal
