#include "Integration.hpp"

// Transitional backend: feature callers no longer depend on generated types.
#include "../SDK/Engine_classes.hpp"

#include <cstring>

namespace p3r::unreal {
namespace {
bool ready = false;
std::optional<ConsoleResult> completed;
}

InitStatus InitializeConsoleRuntime(RuntimeAddresses addresses) noexcept {
    ready = false;
    completed.reset();
    if (!addresses.objectArray) return InitStatus::MissingObjectArray;
    if (!addresses.appendName) return InitStatus::MissingNameAppender;
    SDK::UObject::GObjects.InitManually(addresses.objectArray);
    SDK::FName::InitManually(addresses.appendName);
    ready = true;
    return InitStatus::Ready;
}

std::optional<RenderTargetInfo> ReadRenderTarget(const void* object) noexcept {
    if (!object) return std::nullopt;
    const auto* target = static_cast<const SDK::UTextureRenderTarget2D*>(object);
    return RenderTargetInfo{target->SizeX, target->SizeY,
        target->RenderTargetFormat == SDK::ETextureRenderTargetFormat::RTF_RGBA16f};
}

bool SetRenderTargetSize(void* object, std::int32_t width, std::int32_t height) noexcept {
    if (!object) return false;
    auto* target = static_cast<SDK::UTextureRenderTarget2D*>(object);
    target->SizeX = width;
    target->SizeY = height;
    return true;
}

bool SetCaptureSize(void* object, std::int32_t width, std::int32_t height) noexcept {
    if (!object) return false;
    // Inherited RTCaptureMidHook offsets; moved into private layouts at promotion.
    auto* bytes = static_cast<std::byte*>(object);
    std::memcpy(bytes + 0x1FC, &width, sizeof(width));
    std::memcpy(bytes + 0x200, &height, sizeof(height));
    return true;
}

ConsoleResult TryEnableConsole() {
    if (completed) return *completed;
    ConsoleResult result;
    if (!ready) {
        result.status = ConsoleStatus::RuntimeUnavailable;
        result.diagnostic = "Object registry or name appender is unavailable.";
        return result;
    }
    auto* engineClass = SDK::UObject::FindClassFast("Engine");
    if (!engineClass) return result;
    SDK::UEngine* engine = nullptr;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i) {
        auto* object = SDK::UObject::GObjects->GetByIndex(i);
        if (object && object->IsA(engineClass) && !object->IsDefaultObject()) {
            engine = static_cast<SDK::UEngine*>(object);
            break;
        }
    }
    if (!engine) return result;
    result.engineAddress = reinterpret_cast<std::uintptr_t>(engine);
    if (!engine->ConsoleClass || !engine->GameViewport) return result;

    auto* gameplayClass = SDK::UObject::FindClassFast("GameplayStatics");
    auto* spawn = gameplayClass ? gameplayClass->GetFunction("GameplayStatics", "SpawnObject") : nullptr;
    auto* receiver = gameplayClass ? gameplayClass->DefaultObject : nullptr;
    if (!gameplayClass || !spawn || !receiver || !receiver->VTable ||
        !static_cast<void**>(receiver->VTable)[SDK::Offsets::ProcessEventIdx]) {
        result.status = ConsoleStatus::LookupFailed;
        result.diagnostic = "GameplayStatics class, SpawnObject, default object, or ProcessEvent table entry is unavailable.";
        return result;
    }
    auto* console = SDK::UGameplayStatics::SpawnObject(engine->ConsoleClass, engine->GameViewport);
    if (!console) {
        result.status = ConsoleStatus::SpawnFailed;
        result.diagnostic = "SpawnObject returned no console object.";
        return result;
    }
    engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(console);
    result.status = ConsoleStatus::Enabled;
    auto* inputClass = SDK::UObject::FindClassFast("InputSettings");
    auto* input = inputClass ? static_cast<SDK::UInputSettings*>(inputClass->DefaultObject) : nullptr;
    if (input) {
        if (input->ConsoleKeys && input->ConsoleKeys.Num() > 0) {
            result.keyName = input->ConsoleKeys[0].KeyName.ToString();
            result.keyStatus = result.keyName.empty() ? ConsoleKeyStatus::NameUnavailable : ConsoleKeyStatus::Available;
        } else {
            result.keyStatus = ConsoleKeyStatus::Unbound;
        }
    }
    completed = result;
    return result;
}

} // namespace p3r::unreal
