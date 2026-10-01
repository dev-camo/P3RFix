#include "Runtime.hpp"

#include "Layouts.hpp"
#include "UtfN.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace p3r::unreal::detail {
namespace {

// Lookup and dispatch are extracted from SDK::UObject::FindObjectFastImpl,
// SDK::UStruct::IsSubclassOf, SDK::UClass::GetFunction, SDK::UEngine::GetEngine,
// SDK::FName::ToString and SDK::UGameplayStatics::SpawnObject. All objects and
// metadata remain game-owned. Only the addresses/result below and the name
// scratch allocation belong to the fix. InitializeRuntime resets every cache.
struct RuntimeState {
    RuntimeAddresses addresses{};
    bool ready = false;
    void* engineClass = nullptr;
    void* gameplayClass = nullptr;
    void* inputClass = nullptr;
    void* spawnFunction = nullptr;
    void* engine = nullptr;
    std::optional<ConsoleResult> enabledResult;
};
RuntimeState runtime;

std::optional<ObjectArray> ReadRegistry() noexcept {
    if (!runtime.ready || !runtime.addresses.objectArray)
        return std::nullopt;

    const auto array = ReadMemory<ObjectArray>(runtime.addresses.objectArray, 0);
    if (array.numElements < 0 || array.maxElements < 0 ||
        array.numChunks < 0 || array.maxChunks < 0 ||
        array.numElements > array.maxElements || array.numChunks > array.maxChunks)
        return std::nullopt;

    // Promote before multiplication: malformed game metadata must not overflow.
    if (static_cast<std::int64_t>(array.numElements) >
            static_cast<std::int64_t>(array.numChunks) * ElementsPerChunk ||
        static_cast<std::int64_t>(array.maxElements) >
            static_cast<std::int64_t>(array.maxChunks) * ElementsPerChunk ||
        (array.numChunks != 0 && !array.chunks))
        return std::nullopt;
    return array;
}

void* ObjectAtIndex(const ObjectArray& array, std::int32_t index) noexcept {
    if (index < 0 || index >= array.numElements || !array.chunks)
        return nullptr;
    const auto chunkIndex = index / ElementsPerChunk;
    const auto inChunkIndex = index % ElementsPerChunk;
    if (chunkIndex >= array.numChunks)
        return nullptr;
    void* chunk = ReadMemory<void*>(array.chunks,
                                   static_cast<std::size_t>(chunkIndex) * sizeof(void*));
    if (!chunk)
        return nullptr; // A null chunk or entry is a normal registry hole.
    return ReadMemory<void*>(chunk, static_cast<std::size_t>(inChunkIndex) *
                                     sizeof(ObjectItem) + offsetof(ObjectItem, object));
}

bool HasCastFlag(const void* object, std::uint64_t mask) noexcept {
    if (!object)
        return false;
    const auto objectClass = ReadMemory<void*>(object, offsetof(Object, objectClass));
    if (!objectClass)
        return false;
    const auto flags = ReadMemory<std::uint64_t>(objectClass, offsetof(Class, castFlags));
    // The original enum operator& means mask inclusion, not a truthy bit test.
    return (flags & mask) == mask;
}

std::optional<std::string> NameToString(const Name& name) {
    if (!runtime.ready || !runtime.addresses.appendName)
        return std::nullopt;

    thread_local NameScratch scratch;
    scratch.Reset();
    if (!scratch.Data())
        return std::nullopt;
    struct ResetScratch {
        NameScratch& scratch;
        ~ResetScratch() { scratch.Reset(); }
    } reset{scratch};

    // This is the inherited SDK::FName::AppendString calling signature. On
    // Windows x64 a reference passes the address of the FString header.
    using AppendName = void (*)(const Name*, ArrayHeader&);
    reinterpret_cast<AppendName>(runtime.addresses.appendName)(&name, scratch.header);
    if (!scratch.HasValidResult())
        return std::nullopt;
    std::string value = UtfN::Utf16StringToUtf8String<std::string>(
        scratch.Data(), scratch.header.count - 1); // FString count includes nul.
    const auto slash = value.rfind('/');
    if (slash != std::string::npos)
        value.erase(0, slash + 1);
    if (value.empty())
        return std::nullopt;
    return value;
}

std::optional<std::string> ObjectName(const void* object) {
    if (!object)
        return std::nullopt;
    return NameToString(ReadMemory<Name>(object, offsetof(Object, name)));
}

struct LookupResult {
    void* object = nullptr;
    bool nameConversionFailed = false;
};

LookupResult FindClass(const ObjectArray& array, std::string_view name) {
    LookupResult result;
    for (std::int32_t index = 0; index < array.numElements; ++index) {
        void* object = ObjectAtIndex(array, index);
        if (!HasCastFlag(object, ClassCastFlag))
            continue;
        const auto objectName = ObjectName(object);
        if (!objectName) {
            result.nameConversionFailed = true;
            continue;
        }
        if (objectName && *objectName == name)
            return {object, false}; // Unrelated failed conversions do not mask a match.
    }
    return result;
}

bool IsSubclassOf(void* derivedClass, void* baseClass, std::int32_t objectCount) noexcept {
    // Class chains contain registry objects. The bound also stops malformed
    // cyclic metadata from turning one startup attempt into an infinite loop.
    for (std::int32_t visited = 0; derivedClass && visited < objectCount; ++visited) {
        if (derivedClass == baseClass)
            return true;
        derivedClass = ReadMemory<void*>(derivedClass, offsetof(Struct, super));
    }
    return false;
}

void* FindEngine(const ObjectArray& array, void* engineClass) noexcept {
    for (std::int32_t index = 0; index < array.numElements; ++index) {
        void* object = ObjectAtIndex(array, index);
        if (!object)
            continue;
        const auto flags = static_cast<std::uint32_t>(
            ReadMemory<std::int32_t>(object, offsetof(Object, flags)));
        if ((flags & ClassDefaultObjectFlag) == ClassDefaultObjectFlag)
            continue;
        const auto objectClass = ReadMemory<void*>(object, offsetof(Object, objectClass));
        if (IsSubclassOf(objectClass, engineClass, array.numElements))
            return object;
    }
    return nullptr;
}

LookupResult FindSpawnFunction(const ObjectArray& array, void* gameplayClass) {
    // The legacy GetFunction("GameplayStatics", "SpawnObject") searches the
    // named class's Children chain and verifies each field's function cast flag.
    LookupResult result;
    void* field = ReadMemory<void*>(gameplayClass, offsetof(Struct, children));
    for (std::int32_t visited = 0; field && visited < array.numElements; ++visited) {
        if (HasCastFlag(field, FunctionCastFlag)) {
            const auto name = ObjectName(field);
            if (name && *name == "SpawnObject")
                return {field, false};
            if (!name)
                result.nameConversionFailed = true;
        }
        field = ReadMemory<void*>(field, offsetof(Field, next));
    }
    return result;
}

ConsoleResult Result(ConsoleStatus status, std::string diagnostic) {
    ConsoleResult result;
    result.status = status;
    result.engineAddress = reinterpret_cast<std::uintptr_t>(runtime.engine);
    result.diagnostic = std::move(diagnostic);
    return result;
}

void ReportConsoleKey(const ObjectArray& array, ConsoleResult& result) {
    if (!runtime.inputClass) {
        const auto lookup = FindClass(array, "InputSettings");
        runtime.inputClass = lookup.object;
        if (!runtime.inputClass && lookup.nameConversionFailed) {
            result.keyStatus = ConsoleKeyStatus::NameUnavailable;
            result.diagnostic = "Console enabled; InputSettings class name conversion failed";
            return;
        }
    }
    if (!runtime.inputClass) {
        result.keyStatus = ConsoleKeyStatus::InputSettingsUnavailable;
        result.diagnostic = "Console enabled; InputSettings class is unavailable";
        return;
    }
    void* inputSettings = ReadMemory<void*>(runtime.inputClass, offsetof(Class, defaultObject));
    if (!inputSettings) {
        result.keyStatus = ConsoleKeyStatus::InputSettingsUnavailable;
        result.diagnostic = "Console enabled; InputSettings default object is unavailable";
        return;
    }
    const auto keys = ReadMemory<ArrayHeader>(inputSettings, offsetof(InputSettingsView, consoleKeys));
    if (!IsValidArray(keys)) {
        result.keyStatus = ConsoleKeyStatus::NameUnavailable;
        result.diagnostic = "Console enabled; console key array metadata is invalid";
        return;
    }
    if (keys.count == 0) {
        result.keyStatus = ConsoleKeyStatus::Unbound;
        result.diagnostic = "Console enabled; no console key is bound";
        return;
    }
    const auto key = NameToString(ReadMemory<Name>(keys.data, offsetof(Key, name)));
    if (!key) {
        result.keyStatus = ConsoleKeyStatus::NameUnavailable;
        result.diagnostic = "Console enabled; console key name conversion is unavailable";
        return;
    }
    result.keyStatus = ConsoleKeyStatus::Available;
    result.keyName = *key;
    result.diagnostic = "Console enabled";
}

class RestoreFunctionFlags final {
public:
    explicit RestoreFunctionFlags(void* function) noexcept
        : function_(function), original_(ReadMemory<std::uint32_t>(
              function, offsetof(Function, functionFlags))) {
        WriteMemory(function_, offsetof(Function, functionFlags), original_ | NativeFunctionFlag);
    }
    ~RestoreFunctionFlags() {
        WriteMemory(function_, offsetof(Function, functionFlags), original_);
    }
    RestoreFunctionFlags(const RestoreFunctionFlags&) = delete;
    RestoreFunctionFlags& operator=(const RestoreFunctionFlags&) = delete;
private:
    void* function_;
    std::uint32_t original_;
};

} // namespace

InitStatus InitializeRuntime(RuntimeAddresses addresses) noexcept {
    runtime = {};
    runtime.addresses = addresses;
    if (!addresses.objectArray)
        return InitStatus::MissingObjectArray;
    if (!addresses.appendName)
        return InitStatus::MissingNameAppender;
    runtime.ready = true;
    return InitStatus::Ready;
}

void* ObjectAtIndex(std::int32_t index) noexcept {
    const auto array = ReadRegistry();
    return array ? ObjectAtIndex(*array, index) : nullptr;
}

ConsoleResult TryEnableConsole() {
    if (!runtime.ready)
        return Result(ConsoleStatus::RuntimeUnavailable,
                      "Console runtime object registry or name appender is unavailable");
    if (runtime.enabledResult)
        return *runtime.enabledResult;
    const auto array = ReadRegistry();
    if (!array)
        return Result(ConsoleStatus::LookupFailed, "Object registry metadata is invalid");

    if (!runtime.engineClass) {
        const auto lookup = FindClass(*array, "Engine");
        runtime.engineClass = lookup.object;
        if (!runtime.engineClass && lookup.nameConversionFailed)
            return Result(ConsoleStatus::LookupFailed, "Engine class name conversion failed");
    }
    if (!runtime.engineClass)
        return Result(ConsoleStatus::Pending, "Engine class is not yet available");
    if (!runtime.engine)
        runtime.engine = FindEngine(*array, runtime.engineClass);
    if (!runtime.engine)
        return Result(ConsoleStatus::Pending, "Engine instance is not yet available");
    const auto consoleClass = ReadMemory<void*>(runtime.engine, offsetof(EngineView, consoleClass));
    if (!consoleClass)
        return Result(ConsoleStatus::Pending, "Engine console class is not yet available");
    void* viewport = ReadMemory<void*>(runtime.engine, offsetof(EngineView, gameViewport));
    if (!viewport)
        return Result(ConsoleStatus::Pending, "Engine viewport is not yet available");

    if (!runtime.gameplayClass) {
        const auto lookup = FindClass(*array, "GameplayStatics");
        runtime.gameplayClass = lookup.object;
        if (!runtime.gameplayClass && lookup.nameConversionFailed)
            return Result(ConsoleStatus::LookupFailed, "GameplayStatics class name conversion failed");
    }
    if (!runtime.gameplayClass)
        return Result(ConsoleStatus::LookupFailed, "GameplayStatics class is unavailable");
    if (!runtime.spawnFunction) {
        const auto lookup = FindSpawnFunction(*array, runtime.gameplayClass);
        runtime.spawnFunction = lookup.object;
        if (!runtime.spawnFunction && lookup.nameConversionFailed)
            return Result(ConsoleStatus::LookupFailed,
                          "GameplayStatics SpawnObject function name conversion failed");
    }
    if (!runtime.spawnFunction)
        return Result(ConsoleStatus::LookupFailed, "GameplayStatics SpawnObject function is unavailable");
    void* receiver = ReadMemory<void*>(runtime.gameplayClass, offsetof(Class, defaultObject));
    if (!receiver)
        return Result(ConsoleStatus::LookupFailed, "GameplayStatics default object is unavailable");
    const auto vtable = ReadMemory<void*>(receiver, offsetof(Object, vtable));
    if (!vtable)
        return Result(ConsoleStatus::LookupFailed, "GameplayStatics default object virtual table is unavailable");
    const auto dispatch = ReadMemory<void*>(vtable, ProcessEventIndex * sizeof(void*));
    if (!dispatch)
        return Result(ConsoleStatus::LookupFailed, "GameplayStatics ProcessEvent table entry is unavailable");

    SpawnObjectParameters parameters{};
    parameters.objectClass = consoleClass;
    parameters.outer = viewport;
    {
        RestoreFunctionFlags restore(runtime.spawnFunction);
        using ProcessEvent = void (*)(const void*, void*, void*);
        reinterpret_cast<ProcessEvent>(dispatch)(receiver, runtime.spawnFunction, &parameters);
    }
    if (!parameters.returnValue)
        return Result(ConsoleStatus::SpawnFailed, "GameplayStatics SpawnObject returned null");
    WriteMemory(viewport, offsetof(ViewportView, viewportConsole), parameters.returnValue);
    auto result = Result(ConsoleStatus::Enabled, "Console enabled");
    ReportConsoleKey(*array, result);
    runtime.enabledResult = result;
    return result;
}

} // namespace p3r::unreal::detail
