#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace p3r::unreal {

struct RuntimeAddresses {
    void* objectArray = nullptr;
    void* appendName = nullptr;
};

enum class InitStatus { Ready, MissingObjectArray, MissingNameAppender };

struct RenderTargetInfo {
    std::int32_t width = 0;
    std::int32_t height = 0;
    bool isRgba16f = false;
};

enum class ConsoleStatus { Pending, Enabled, RuntimeUnavailable, LookupFailed, SpawnFailed };
enum class ConsoleKeyStatus { Available, Unbound, InputSettingsUnavailable, NameUnavailable };

struct ConsoleResult {
    ConsoleStatus status = ConsoleStatus::Pending;
    ConsoleKeyStatus keyStatus = ConsoleKeyStatus::InputSettingsUnavailable;
    std::uintptr_t engineAddress = 0;
    std::string keyName;
    std::string diagnostic;
};

InitStatus InitializeConsoleRuntime(RuntimeAddresses addresses) noexcept;
std::optional<RenderTargetInfo> ReadRenderTarget(const void* object) noexcept;
bool SetRenderTargetSize(void* object, std::int32_t width, std::int32_t height) noexcept;
bool SetCaptureSize(void* object, std::int32_t width, std::int32_t height) noexcept;
ConsoleResult TryEnableConsole();

} // namespace p3r::unreal
