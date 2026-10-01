// Synthetic game-memory fixtures intentionally use literal, independently
// reviewed SDK offsets. They must not inherit the implementation's layouts.
#include "unreal/Integration.hpp"
#include "unreal/detail/Layouts.hpp" // Callback types only; never fixture offsets.
#include "unreal/detail/Runtime.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
using namespace p3r::unreal;
using Byte = std::byte;

static_assert(sizeof(void*) == 8, "Tests require the game's x64 ABI");
static_assert(sizeof(wchar_t) == 2, "Tests require the game's UTF-16 ABI");

void Require(bool condition, const char* explanation)
{
    if (!condition)
        throw std::runtime_error(explanation);
}

template<typename T>
T Read(const void* bytes, std::size_t offset)
{
    T value{};
    std::memcpy(&value, static_cast<const Byte*>(bytes) + offset, sizeof(value));
    return value;
}

template<typename T>
void Write(void* bytes, std::size_t offset, const T& value)
{
    std::memcpy(static_cast<Byte*>(bytes) + offset, &value, sizeof(value));
}

template<std::size_t N>
struct alignas(16) Bytes
{
    std::array<Byte, N> data{};
    void* address() { return data.data(); }
    const void* address() const { return data.data(); }
};

// Feature behavior always uses the public, shipped implementation. Only the
// distinct registry-index boundary checks below call the private helper.
InitStatus Initialize(RuntimeAddresses addresses)
{
    return InitializeConsoleRuntime(addresses);
}

ConsoleResult Enable()
{
    return TryEnableConsole();
}

struct Registry
{
    Bytes<0x20> header;
    std::array<void*, 3> chunks{};
    std::vector<Byte> first = std::vector<Byte>(0x10000 * 0x18);
    std::vector<Byte> second = std::vector<Byte>(0x10000 * 0x18);
    std::vector<Byte> third = std::vector<Byte>(0x10000 * 0x18);

    Registry()
    {
        chunks = {first.data(), second.data(), third.data()};
        Write(header.address(), 0, static_cast<void*>(chunks.data()));
        Counts(0x10000, 0, 1, 1);
    }

    void Counts(std::int32_t maxElements, std::int32_t elements,
                std::int32_t maxChunks, std::int32_t chunkCount)
    {
        Write(header.address(), 0x10, maxElements);
        Write(header.address(), 0x14, elements);
        Write(header.address(), 0x18, maxChunks);
        Write(header.address(), 0x1C, chunkCount);
    }

    void Slot(std::int32_t index, void* object)
    {
        auto* chunk = static_cast<Byte*>(chunks.at(static_cast<std::size_t>(index / 0x10000)));
        Write(chunk, static_cast<std::size_t>(index % 0x10000) * 0x18, object);
    }
};

enum NameIndex : std::int32_t
{
    ClassName = 1, FunctionName, EngineName, GameplayName, InputName,
    DerivedName, DefaultEngineName, LiveEngineName, ConsoleName,
    SpawnName, GameplayDefaultName, InputDefaultName, KeyName, OtherName
};

enum class NameFault
{
    None, Empty, ZeroCount, NegativeCount, ExcessCount, NullData,
    ChangedData, ChangedCapacity, MissingTerminator
};

struct Fixture;
Fixture* active = nullptr;
void AppendName(const detail::Name* name, detail::ArrayHeader& output);
void Dispatch(const void* receiver, void* function, void* parameters);

struct Fixture
{
    Registry registry;
    Bytes<0x230> classClass, functionClass, engineClass, gameplayClass;
    Bytes<0x230> inputClass, derivedClass, consoleClass;
    Bytes<0x788> engineDefault, engine;
    Bytes<0x48> viewport;
    Bytes<0x28> gameplayDefault, console, oldConsole, impostor;
    Bytes<0x140> inputDefault;
    Bytes<0xE0> spawn;
    Bytes<0x30> nonFunctionChild;
    Bytes<0x30> keys;
    std::array<void*, 0x45> vtable{};
    std::array<std::u16string, 32> names{};
    NameFault nameFault = NameFault::None;
    std::int32_t faultyName = KeyName;
    int nameCalls = 0;
    int dispatchCalls = 0;
    bool cleanScratch = true;
    bool stableScratch = true;
    void* scratch = nullptr;
    const void* seenReceiver = nullptr;
    void* seenFunction = nullptr;
    void* seenClass = nullptr;
    void* seenOuter = nullptr;
    void* seenInitialReturn = reinterpret_cast<void*>(1);
    std::uint32_t seenFlags = 0;
    std::uint32_t flags = 0xA012;
    void* spawnReturn = nullptr;
    bool throwDispatch = false;

    Fixture()
    {
        active = this;
        names[ClassName] = u"Class";
        names[FunctionName] = u"Function";
        names[EngineName] = u"Engine";
        names[GameplayName] = u"GameplayStatics";
        names[InputName] = u"InputSettings";
        names[DerivedName] = u"P3REngine";
        names[DefaultEngineName] = u"Default__P3REngine";
        names[LiveEngineName] = u"P3REngine_0";
        names[ConsoleName] = u"Console";
        names[SpawnName] = u"SpawnObject";
        names[GameplayDefaultName] = u"Default__GameplayStatics";
        names[InputDefaultName] = u"Default__InputSettings";
        names[KeyName] = u"Tilde";
        names[OtherName] = u"Unrelated";

        Object(classClass.address(), ClassName, classClass.address());
        Write(classClass.address(), 0xD0, std::uint64_t{0x20});
        Object(functionClass.address(), FunctionName, classClass.address());
        Write(functionClass.address(), 0xD0, std::uint64_t{0x80000});
        Object(engineClass.address(), EngineName, classClass.address());
        Object(gameplayClass.address(), GameplayName, classClass.address());
        Object(inputClass.address(), InputName, classClass.address());
        Object(derivedClass.address(), DerivedName, classClass.address());
        Object(consoleClass.address(), ConsoleName, classClass.address());
        Write(derivedClass.address(), 0x40, engineClass.address());
        Object(engineDefault.address(), DefaultEngineName, derivedClass.address());
        Write(engineDefault.address(), 8, std::int32_t{0x10});
        Object(engine.address(), LiveEngineName, derivedClass.address());
        Write(engine.address(), 0xF0, consoleClass.address());
        Write(engine.address(), 0x780, viewport.address());
        Write(viewport.address(), 0x40, oldConsole.address());
        Object(gameplayDefault.address(), GameplayDefaultName, gameplayClass.address());
        Write(gameplayDefault.address(), 0, static_cast<void*>(vtable.data()));
        Write(gameplayClass.address(), 0x118, gameplayDefault.address());
        vtable[0x44] = reinterpret_cast<void*>(&Dispatch);
        Object(spawn.address(), SpawnName, functionClass.address());
        Write(spawn.address(), 0xB0, flags);
        // An identically named non-function precedes the actual reflected call.
        Object(nonFunctionChild.address(), SpawnName, classClass.address());
        Write(nonFunctionChild.address(), 0x28, spawn.address());
        Write(gameplayClass.address(), 0x48, nonFunctionChild.address());
        Object(inputDefault.address(), InputDefaultName, inputClass.address());
        Write(inputClass.address(), 0x118, inputDefault.address());
        Write(keys.address(), 0, std::int32_t{KeyName});
        SetKeys(keys.address(), 1, 2);
        // An identically named non-class precedes the actual engine metadata.
        Object(impostor.address(), EngineName, functionClass.address());
        const std::array<void*, 16> objects = {
            impostor.address(), classClass.address(), functionClass.address(),
            engineClass.address(), gameplayClass.address(), inputClass.address(),
            derivedClass.address(), engineDefault.address(), engine.address(),
            consoleClass.address(), nonFunctionChild.address(), spawn.address(),
            gameplayDefault.address(), inputDefault.address(), viewport.address(), console.address()
        };
        for (std::size_t i = 0; i < objects.size(); ++i)
        {
            registry.Slot(static_cast<std::int32_t>(i), objects[i]);
            Write(objects[i], 0x0C, static_cast<std::int32_t>(i));
        }
        registry.Counts(0x10000, static_cast<std::int32_t>(objects.size()), 1, 1);
        spawnReturn = console.address();
    }

    ~Fixture() { active = nullptr; }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;

    void Object(void* address, std::int32_t name, void* type)
    {
        Write(address, 0x10, type);
        Write(address, 0x18, name);
    }

    void SetKeys(void* data, std::int32_t count, std::int32_t capacity)
    {
        Write(inputDefault.address(), 0x130, data);
        Write(inputDefault.address(), 0x138, count);
        Write(inputDefault.address(), 0x13C, capacity);
    }

    void Start()
    {
        Require(Initialize({registry.header.address(), reinterpret_cast<void*>(&AppendName)}) == InitStatus::Ready,
                "complete dependencies must initialize");
    }

    void RequireNoSpawn(ConsoleStatus expected)
    {
        const auto result = Enable();
        Require(result.status == expected, "unexpected guarded status");
        Require(!result.diagnostic.empty(), "guarded result needs a diagnostic");
        Require(dispatchCalls == 0, "guarded lookup must not dispatch");
        Require(Read<void*>(viewport.address(), 0x40) == oldConsole.address(), "guarded lookup must not replace viewport console");
        Require(Read<std::uint32_t>(spawn.address(), 0xB0) == flags, "guarded lookup must preserve function flags");
    }
};

void AppendName(const detail::Name* name, detail::ArrayHeader& outputHeader)
{
    void* output = &outputHeader;
    Require(active != nullptr, "name callback needs its fixture");
    auto& fixture = *active;
    ++fixture.nameCalls;
    auto* data = Read<char16_t*>(output, 0);
    fixture.cleanScratch = fixture.cleanScratch && Read<std::int32_t>(output, 8) == 0
                           && Read<std::int32_t>(output, 12) == 1024 && data != nullptr;
    if (fixture.scratch)
        fixture.stableScratch = fixture.stableScratch && fixture.scratch == data;
    else
        fixture.scratch = data;

    const auto index = Read<std::int32_t>(name, 0);
    auto text = fixture.names.at(static_cast<std::size_t>(index));
    const auto number = Read<std::uint32_t>(name, 4);
    if (number)
    {
        text += u'_';
        for (const char c : std::to_string(number - 1))
            text += static_cast<char16_t>(c);
    }
    const auto fault = index == fixture.faultyName ? fixture.nameFault : NameFault::None;
    if (fault == NameFault::Empty)
        text.clear();
    Require(text.size() < 1024, "fixture name exceeds supplied scratch capacity");
    std::memcpy(data, text.data(), text.size() * sizeof(char16_t));
    data[text.size()] = 0;
    Write(output, 8, static_cast<std::int32_t>(text.size() + 1));
    switch (fault)
    {
    case NameFault::ZeroCount: Write(output, 8, std::int32_t{0}); break;
    case NameFault::NegativeCount: Write(output, 8, std::int32_t{-1}); break;
    case NameFault::ExcessCount: Write(output, 8, std::int32_t{1025}); break;
    case NameFault::NullData: Write(output, 0, static_cast<void*>(nullptr)); break;
    case NameFault::ChangedData:
    {
        static char16_t borrowed[] = u"Tilde";
        Write(output, 0, static_cast<void*>(borrowed));
        break;
    }
    case NameFault::ChangedCapacity: Write(output, 12, std::int32_t{1023}); break;
    case NameFault::MissingTerminator: data[text.size()] = u'X'; break;
    default: break;
    }
}

void Dispatch(const void* receiver, void* function, void* parameters)
{
    Require(active != nullptr, "dispatch callback needs its fixture");
    auto& fixture = *active;
    ++fixture.dispatchCalls;
    fixture.seenReceiver = receiver;
    fixture.seenFunction = function;
    fixture.seenClass = Read<void*>(parameters, 0);
    fixture.seenOuter = Read<void*>(parameters, 8);
    fixture.seenInitialReturn = Read<void*>(parameters, 16);
    fixture.seenFlags = Read<std::uint32_t>(function, 0xB0);
    if (fixture.throwDispatch)
        throw std::runtime_error("synthetic ProcessEvent failure");
    Write(parameters, 16, fixture.spawnReturn);
}

void ExpectKey(Fixture& fixture, ConsoleKeyStatus expected, const std::string& key = {})
{
    fixture.Start();
    const auto result = Enable();
    Require(result.status == ConsoleStatus::Enabled, "key reporting must leave console enabled");
    Require(result.keyStatus == expected, "unexpected console key reporting status");
    Require(result.keyName == key, "unexpected console key text");
    Require(fixture.dispatchCalls == 1, "key reporting must follow one spawn");
    Require(Read<void*>(fixture.viewport.address(), 0x40) == fixture.console.address(), "key reporting must retain created console");
    Require(fixture.cleanScratch, "name conversion must reset count and restore supplied metadata");
    Require(fixture.stableScratch, "name conversion must reuse fix-owned storage");
}

void RequireNameFailureDiagnostic(const ConsoleResult& result)
{
    Require(result.diagnostic.find("name conversion") != std::string::npos,
            "conversion failure diagnostic must identify failed name conversion");
}

void RequireOnlyChanges(const std::vector<Byte>& before, const void* after,
                        std::size_t firstOffset, std::size_t secondOffset)
{
    const auto* bytes = static_cast<const Byte*>(after);
    for (std::size_t i = 0; i < before.size(); ++i)
    {
        const bool field = (i >= firstOffset && i < firstOffset + 4)
                           || (i >= secondOffset && i < secondOffset + 4);
        Require(field || before[i] == bytes[i], "write changed a neighboring sentinel byte");
    }
}

using Test = std::pair<const char*, std::function<void()>>;
std::vector<Test> Tests()
{
    std::vector<Test> tests;
    tests.emplace_back("render reads independent offsets and RGBA16f", [] {
        Bytes<0x210> target;
        std::fill(target.data.begin(), target.data.end(), Byte{0xA5});
        Write(target.address(), 0x180, std::int32_t{1920});
        Write(target.address(), 0x184, std::int32_t{1080});
        Write(target.address(), 0x19B, std::uint8_t{6});
        const auto info = ReadRenderTarget(target.address());
        Require(info && info->width == 1920 && info->height == 1080 && info->isRgba16f,
                "render fields must match the independent byte fixture");
        Write(target.address(), 0x19B, std::uint8_t{5});
        Require(!ReadRenderTarget(target.address())->isRgba16f, "another format must not be RGBA16f");
    });
    tests.emplace_back("render size writes preserve every other byte", [] {
        Bytes<0x210> target;
        std::fill(target.data.begin(), target.data.end(), Byte{0xA5});
        const std::vector<Byte> before(target.data.begin(), target.data.end());
        Require(SetRenderTargetSize(target.address(), 2560, 1440), "render resize must succeed");
        Require(Read<std::int32_t>(target.address(), 0x180) == 2560 && Read<std::int32_t>(target.address(), 0x184) == 1440,
                "render resize must write independent size offsets");
        RequireOnlyChanges(before, target.address(), 0x180, 0x184);
    });
    tests.emplace_back("capture size writes preserve every other byte", [] {
        Bytes<0x210> capture;
        std::fill(capture.data.begin(), capture.data.end(), Byte{0x5A});
        const std::vector<Byte> before(capture.data.begin(), capture.data.end());
        Require(SetCaptureSize(capture.address(), 3200, 1800), "capture resize must succeed");
        Require(Read<std::int32_t>(capture.address(), 0x1FC) == 3200 && Read<std::int32_t>(capture.address(), 0x200) == 1800,
                "capture resize must write independent size offsets");
        RequireOnlyChanges(before, capture.address(), 0x1FC, 0x200);
    });
    tests.emplace_back("null render and capture operations fail safely", [] {
        Require(!ReadRenderTarget(nullptr), "null render read must fail");
        Require(!SetRenderTargetSize(nullptr, 1, 1), "null render write must fail");
        Require(!SetCaptureSize(nullptr, 1, 1), "null capture write must fail");
    });
    tests.emplace_back("render works independently of missing console runtime", [] {
        Require(Initialize({}) == InitStatus::MissingObjectArray, "missing registry has precedence");
        Bytes<0x210> target;
        Require(SetRenderTargetSize(target.address(), 640, 480), "render cannot depend on console registry");
        Require(ReadRenderTarget(target.address())->width == 640, "independent render write must remain readable");
    });

    tests.emplace_back("registry crosses 0xFFFF to 0x10000 with holes", [] {
        Fixture fixture;
        fixture.registry.Counts(0x20000, 0x10002, 2, 2);
        fixture.registry.Slot(0xFFFF, fixture.engine.address());
        fixture.registry.Slot(0x10000, fixture.viewport.address());
        fixture.Start();
        Require(detail::ObjectAtIndex(0xFFFF) == fixture.engine.address(), "last first-chunk item mismatch");
        Require(detail::ObjectAtIndex(0x10000) == fixture.viewport.address(), "first second-chunk item mismatch");
        Require(detail::ObjectAtIndex(0x10001) == nullptr, "null entry is an ordinary hole");
    });
    tests.emplace_back("registry skips a null chunk and finds later chunk", [] {
        Fixture fixture;
        fixture.registry.Counts(0x30000, 0x20001, 3, 3);
        fixture.registry.chunks[1] = nullptr;
        fixture.registry.Slot(0x20000, fixture.engine.address());
        fixture.Start();
        Require(detail::ObjectAtIndex(0x10000) == nullptr, "null chunk must be a hole");
        Require(detail::ObjectAtIndex(0x20000) == fixture.engine.address(), "later nonnull chunk must be reachable");
    });
    tests.emplace_back("console lookup traverses null entries and chunks", [] {
        Fixture fixture;
        fixture.registry.Counts(0x30000, 0x20001, 3, 3);
        fixture.registry.Slot(8, nullptr);
        fixture.registry.chunks[1] = nullptr;
        fixture.registry.Slot(0x20000, fixture.engine.address());
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::Enabled, "lookup must continue through registry holes");
        Require(result.engineAddress == reinterpret_cast<std::uintptr_t>(fixture.engine.address()), "lookup must find engine in the later chunk");
        Require(fixture.dispatchCalls == 1, "lookup through holes must spawn once");
    });
    tests.emplace_back("empty registry returns pending without callbacks", [] {
        Fixture fixture;
        fixture.registry.Counts(0, 0, 0, 0);
        fixture.Start();
        Require(detail::ObjectAtIndex(0) == nullptr, "empty registry has no first entry");
        fixture.RequireNoSpawn(ConsoleStatus::Pending);
        Require(fixture.nameCalls == 0, "empty registry must not convert names");
    });
    tests.emplace_back("registry rejects negative and end indices", [] {
        Fixture fixture;
        fixture.Start();
        Require(detail::ObjectAtIndex(-1) == nullptr, "negative index must fail");
        Require(detail::ObjectAtIndex(16) == nullptr, "index at element count must fail");
        Require(detail::ObjectAtIndex(0x7FFFFFFF) == nullptr, "large invalid index must fail");
    });
    tests.emplace_back("registry rejects null chunk table", [] {
        Fixture fixture;
        Write(fixture.registry.header.address(), 0, static_cast<void*>(nullptr));
        fixture.Start();
        Require(detail::ObjectAtIndex(0) == nullptr, "null table must fail without dereference");
        Require(Enable().status != ConsoleStatus::Enabled && fixture.dispatchCalls == 0, "null table cannot dispatch");
    });
    const std::array<std::pair<const char*, std::array<std::int32_t, 4>>, 9> invalidCounts = {{
        {"registry rejects negative element count", {0x10000, -1, 1, 1}},
        {"registry rejects negative element capacity", {-1, 0, 1, 1}},
        {"registry rejects elements above capacity", {4, 5, 1, 1}},
        {"registry rejects negative chunk count", {0x10000, 1, 1, -1}},
        {"registry rejects negative chunk capacity", {0x10000, 1, -1, 0}},
        {"registry rejects chunks above capacity", {0x20000, 1, 1, 2}},
        {"registry rejects elements above chunk coverage", {0x20000, 0x10001, 2, 1}},
        {"registry rejects capacity above maximum chunk coverage", {0x20000, 1, 1, 1}},
        {"registry rejects nonempty count without chunks", {0x10000, 1, 1, 0}}
    }};
    for (const auto& [name, counts] : invalidCounts)
    {
        tests.emplace_back(name, [counts] {
            Fixture fixture;
            fixture.registry.Counts(counts[0], counts[1], counts[2], counts[3]);
            fixture.Start();
            Require(detail::ObjectAtIndex(0) == nullptr, "inconsistent registry must reject traversal");
            const auto result = Enable();
            Require(result.status != ConsoleStatus::Enabled, "inconsistent registry cannot enable console");
            Require(fixture.nameCalls == 0 && fixture.dispatchCalls == 0, "invalid counts must prevent callbacks");
        });
    }

    tests.emplace_back("missing object registry prevents callbacks", [] {
        Fixture fixture;
        Require(Initialize({nullptr, reinterpret_cast<void*>(&AppendName)}) == InitStatus::MissingObjectArray, "registry status mismatch");
        fixture.RequireNoSpawn(ConsoleStatus::RuntimeUnavailable);
        Require(fixture.nameCalls == 0, "missing registry must not call name appender");
        Require(detail::ObjectAtIndex(0) == nullptr, "missing registry has no objects");
    });
    tests.emplace_back("missing name appender prevents callbacks", [] {
        Fixture fixture;
        Require(Initialize({fixture.registry.header.address(), nullptr}) == InitStatus::MissingNameAppender, "name status mismatch");
        fixture.RequireNoSpawn(ConsoleStatus::RuntimeUnavailable);
        Require(fixture.nameCalls == 0, "missing appender must not convert names");
    });
    tests.emplace_back("missing both dependencies reports registry first", [] {
        Fixture fixture;
        Require(Initialize({}) == InitStatus::MissingObjectArray, "registry must be reported first");
        fixture.RequireNoSpawn(ConsoleStatus::RuntimeUnavailable);
        Require(fixture.nameCalls == 0, "missing dependencies must prevent conversions");
    });
    tests.emplace_back("derived engine excludes its class default object", [] {
        Fixture fixture;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::Enabled, "derived engine must enable console");
        Require(result.engineAddress == reinterpret_cast<std::uintptr_t>(fixture.engine.address()), "live derived engine must be selected");
        Require(Read<void*>(fixture.engineDefault.address(), 0x780) == nullptr, "default object must remain untouched");
    });
    tests.emplace_back("type and default-object masks tolerate additional flags", [] {
        Fixture fixture;
        Write(fixture.classClass.address(), 0xD0, std::uint64_t{0x21});
        Write(fixture.functionClass.address(), 0xD0, std::uint64_t{0x80001});
        Write(fixture.engineDefault.address(), 8, std::int32_t{0x11});
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::Enabled, "mask inclusion must tolerate unrelated flag bits");
        Require(result.engineAddress == reinterpret_cast<std::uintptr_t>(fixture.engine.address()), "additional default flags must still exclude template engine");
        Require(fixture.seenFunction == fixture.spawn.address(), "additional function flags must still identify SpawnObject");
    });
    tests.emplace_back("spawn uses exact receiver parameters and native flags", [] {
        Fixture fixture;
        fixture.Start();
        Require(Enable().status == ConsoleStatus::Enabled, "healthy spawn must enable");
        Require(fixture.seenReceiver == fixture.gameplayDefault.address(), "receiver must be GameplayStatics default object");
        Require(fixture.seenFunction == fixture.spawn.address(), "only function metadata may dispatch");
        Require(fixture.seenClass == fixture.consoleClass.address(), "parameter offset zero must hold ConsoleClass");
        Require(fixture.seenOuter == fixture.viewport.address(), "parameter offset eight must hold viewport");
        Require(fixture.seenInitialReturn == nullptr, "parameter return slot must be zero initialized");
        Require(fixture.seenFlags == (fixture.flags | 0x400), "native bit must be set during dispatch");
        Require(Read<std::uint32_t>(fixture.spawn.address(), 0xB0) == fixture.flags, "flags must restore exactly");
        Require(Read<void*>(fixture.viewport.address(), 0x40) == fixture.console.address(), "returned pointer at sixteen must become viewport console");
    });
    tests.emplace_back("already native flags restore exactly", [] {
        Fixture fixture;
        fixture.flags |= 0x400;
        Write(fixture.spawn.address(), 0xB0, fixture.flags);
        fixture.Start();
        Require(Enable().status == ConsoleStatus::Enabled, "native spawn must succeed");
        Require(Read<std::uint32_t>(fixture.spawn.address(), 0xB0) == fixture.flags, "existing native flags must survive");
    });
    tests.emplace_back("null spawn return leaves viewport unchanged", [] {
        Fixture fixture;
        fixture.spawnReturn = nullptr;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::SpawnFailed && !result.diagnostic.empty(), "null return needs spawn failure diagnostic");
        Require(fixture.dispatchCalls == 1, "null spawn must dispatch exactly once");
        Require(Read<void*>(fixture.viewport.address(), 0x40) == fixture.oldConsole.address(), "null result must preserve viewport");
        Require(Read<std::uint32_t>(fixture.spawn.address(), 0xB0) == fixture.flags, "null spawn must restore flags");
    });
    tests.emplace_back("throwing dispatch restores flags before propagation", [] {
        Fixture fixture;
        fixture.throwDispatch = true;
        fixture.Start();
        bool caught = false;
        try { (void)Enable(); }
        catch (const std::runtime_error&) { caught = true; }
        Require(caught, "synthetic dispatch exception must reach test boundary");
        Require(fixture.dispatchCalls == 1, "throwing dispatch must be called once");
        Require(Read<std::uint32_t>(fixture.spawn.address(), 0xB0) == fixture.flags, "unwinding must restore original flags");
        Require(Read<void*>(fixture.viewport.address(), 0x40) == fixture.oldConsole.address(), "throwing dispatch must not assign console");
    });
    tests.emplace_back("successful retries create only one console", [] {
        Fixture fixture;
        fixture.Start();
        const auto first = Enable();
        const auto second = Enable();
        Require(first.status == ConsoleStatus::Enabled && second.status == ConsoleStatus::Enabled, "repeat result must stay enabled");
        Require(first.engineAddress == second.engineAddress && first.keyName == second.keyName, "cached result must be stable");
        Require(fixture.dispatchCalls == 1, "repeat startup attempt must not spawn twice");
    });
    tests.emplace_back("reinitialization clears completed result and lookup caches", [] {
        Fixture first;
        first.Start();
        Require(Enable().status == ConsoleStatus::Enabled, "first fixture must enable");
        Fixture second;
        second.names[KeyName] = u"F10";
        second.Start();
        const auto result = Enable();
        Require(result.engineAddress == reinterpret_cast<std::uintptr_t>(second.engine.address()), "reinitialization must forget first engine");
        Require(result.keyName == "F10" && second.dispatchCalls == 1, "reinitialization must use new metadata and spawn");
    });
    tests.emplace_back("failed reinitialization invalidates earlier success", [] {
        Fixture fixture;
        fixture.Start();
        Require(Enable().status == ConsoleStatus::Enabled, "initial success required");
        Require(Initialize({}) == InitStatus::MissingObjectArray, "failed reinitialize must report missing registry");
        Require(Enable().status == ConsoleStatus::RuntimeUnavailable, "failed reinitialize cannot return cached success");
        Require(fixture.dispatchCalls == 1, "failed reinitialize must not dispatch again");
    });

    const std::array<std::pair<const char*, std::function<void(Fixture&)>>, 4> pendingCases = {{
        {"missing engine metadata remains pending", [](Fixture& f) { f.registry.Slot(3, nullptr); }},
        {"missing live engine remains pending despite default object", [](Fixture& f) { f.registry.Slot(8, nullptr); }},
        {"missing console class remains pending", [](Fixture& f) { Write(f.engine.address(), 0xF0, static_cast<void*>(nullptr)); }},
        {"missing viewport remains pending", [](Fixture& f) { Write(f.engine.address(), 0x780, static_cast<void*>(nullptr)); }}
    }};
    for (const auto& [name, change] : pendingCases)
        tests.emplace_back(name, [change] { Fixture fixture; change(fixture); fixture.Start(); fixture.RequireNoSpawn(ConsoleStatus::Pending); });
    tests.emplace_back("pending engine becomes available on later attempt", [] {
        Fixture fixture;
        fixture.registry.Slot(8, nullptr);
        fixture.Start();
        fixture.RequireNoSpawn(ConsoleStatus::Pending);
        fixture.registry.Slot(8, fixture.engine.address());
        Require(Enable().status == ConsoleStatus::Enabled && fixture.dispatchCalls == 1, "later legitimate engine must allow success");
    });
    tests.emplace_back("pending viewport becomes available on later attempt", [] {
        Fixture fixture;
        Write(fixture.engine.address(), 0x780, static_cast<void*>(nullptr));
        fixture.Start();
        fixture.RequireNoSpawn(ConsoleStatus::Pending);
        Write(fixture.engine.address(), 0x780, fixture.viewport.address());
        Require(Enable().status == ConsoleStatus::Enabled && fixture.dispatchCalls == 1, "later legitimate viewport must allow success");
    });
    const std::array<std::pair<const char*, std::function<void(Fixture&)>>, 6> lookupCases = {{
        {"missing GameplayStatics class prevents dispatch", [](Fixture& f) { f.registry.Slot(4, nullptr); }},
        {"missing SpawnObject function prevents dispatch", [](Fixture& f) { Write(f.nonFunctionChild.address(), 0x28, static_cast<void*>(nullptr)); }},
        {"missing GameplayStatics default object prevents dispatch", [](Fixture& f) { Write(f.gameplayClass.address(), 0x118, static_cast<void*>(nullptr)); }},
        {"missing receiver virtual table prevents dispatch", [](Fixture& f) { Write(f.gameplayDefault.address(), 0, static_cast<void*>(nullptr)); }},
        {"missing ProcessEvent entry prevents dispatch", [](Fixture& f) { f.vtable[0x44] = nullptr; }},
        {"wrong function type flag prevents dispatch", [](Fixture& f) { Write(f.functionClass.address(), 0xD0, std::uint64_t{0}); }}
    }};
    for (const auto& [name, change] : lookupCases)
        tests.emplace_back(name, [change] { Fixture fixture; change(fixture); fixture.Start(); fixture.RequireNoSpawn(ConsoleStatus::LookupFailed); });

    tests.emplace_back("bound console key reports its name", [] { Fixture fixture; ExpectKey(fixture, ConsoleKeyStatus::Available, "Tilde"); });
    tests.emplace_back("empty console key array reports unbound", [] {
        Fixture fixture;
        fixture.SetKeys(nullptr, 0, 0);
        ExpectKey(fixture, ConsoleKeyStatus::Unbound);
    });
    tests.emplace_back("missing InputSettings class leaves console enabled", [] {
        Fixture fixture;
        fixture.registry.Slot(5, nullptr);
        ExpectKey(fixture, ConsoleKeyStatus::InputSettingsUnavailable);
    });
    tests.emplace_back("missing InputSettings default object leaves console enabled", [] {
        Fixture fixture;
        Write(fixture.inputClass.address(), 0x118, static_cast<void*>(nullptr));
        ExpectKey(fixture, ConsoleKeyStatus::InputSettingsUnavailable);
    });
    tests.emplace_back("invalid console key array leaves console enabled", [] {
        Fixture fixture;
        fixture.SetKeys(fixture.keys.address(), 3, 2);
        ExpectKey(fixture, ConsoleKeyStatus::NameUnavailable);
    });
    tests.emplace_back("null nonempty console key array leaves console enabled", [] {
        Fixture fixture;
        fixture.SetKeys(nullptr, 1, 1);
        ExpectKey(fixture, ConsoleKeyStatus::NameUnavailable);
    });
    tests.emplace_back("name conversion strips only the final slash prefix", [] {
        Fixture fixture;
        fixture.names[EngineName] = u"/Script/Engine/Engine";
        fixture.names[GameplayName] = u"/Script/Engine/GameplayStatics";
        fixture.names[KeyName] = u"/Input/Keys/F9";
        ExpectKey(fixture, ConsoleKeyStatus::Available, "F9");
    });
    tests.emplace_back("numbered name passes Number to game appender", [] {
        Fixture fixture;
        Write(fixture.keys.address(), 4, std::uint32_t{3});
        ExpectKey(fixture, ConsoleKeyStatus::Available, "Tilde_2");
    });
    tests.emplace_back("name conversion preserves Unicode and surrogate pair", [] {
        Fixture fixture;
        fixture.names[KeyName] = u"\u00E9\u2605\U0001F642";
        ExpectKey(fixture, ConsoleKeyStatus::Available, "\xC3\xA9\xE2\x98\x85\xF0\x9F\x99\x82");
    });
    tests.emplace_back("repeated conversions reuse scratch without stale text", [] {
        Fixture fixture;
        fixture.names[EngineName] = u"/A/VeryLongPrefix/Engine";
        fixture.names[GameplayName] = u"/AnEvenLongerPrefix/GameplayStatics";
        fixture.names[KeyName] = u"K";
        ExpectKey(fixture, ConsoleKeyStatus::Available, "K");
        Require(fixture.nameCalls > 4, "fixture must exercise repeated name conversion");
    });
    const std::array<std::pair<const char*, NameFault>, 9> faults = {{
        {"empty converted key name reports unavailable", NameFault::Empty},
        {"zero returned string count reports unavailable", NameFault::ZeroCount},
        {"negative returned string count reports unavailable", NameFault::NegativeCount},
        {"count exceeding scratch capacity reports unavailable", NameFault::ExcessCount},
        {"null returned string data reports unavailable", NameFault::NullData},
        {"replaced scratch pointer reports unavailable without freeing it", NameFault::ChangedData},
        {"changed scratch capacity reports unavailable", NameFault::ChangedCapacity},
        {"unterminated returned string reports unavailable", NameFault::MissingTerminator},
        {"trailing slash produces unavailable empty key name", NameFault::None}
    }};
    for (const auto& [name, fault] : faults)
    {
        tests.emplace_back(name, [fault] {
            Fixture fixture;
            fixture.nameFault = fault;
            if (fault == NameFault::None)
                fixture.names[KeyName] = u"/Input/";
            ExpectKey(fixture, ConsoleKeyStatus::NameUnavailable);
            Require(!Enable().keyName.size(), "unavailable key result must not retain text");
            Require(fixture.dispatchCalls == 1, "key failure must still cache successful console creation");
        });
    }
    tests.emplace_back("invalid Engine metadata conversion reports lookup failure", [] {
        Fixture fixture;
        fixture.faultyName = EngineName;
        fixture.nameFault = NameFault::ChangedCapacity;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::LookupFailed, "invalid engine metadata cannot be treated as pending startup");
        RequireNameFailureDiagnostic(result);
        Require(fixture.dispatchCalls == 0, "invalid metadata conversion cannot dispatch");
        Require(fixture.cleanScratch && fixture.stableScratch, "failed conversion must restore scratch before further lookups");
    });
    tests.emplace_back("invalid GameplayStatics metadata conversion reports lookup failure", [] {
        Fixture fixture;
        fixture.faultyName = GameplayName;
        fixture.nameFault = NameFault::ChangedCapacity;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::LookupFailed, "invalid gameplay metadata needs lookup failure");
        RequireNameFailureDiagnostic(result);
        Require(fixture.dispatchCalls == 0, "invalid gameplay conversion cannot dispatch");
        Require(Read<void*>(fixture.viewport.address(), 0x40) == fixture.oldConsole.address(), "invalid gameplay conversion must preserve viewport");
    });
    tests.emplace_back("invalid SpawnObject metadata conversion reports lookup failure", [] {
        Fixture fixture;
        fixture.faultyName = SpawnName;
        fixture.nameFault = NameFault::ChangedCapacity;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::LookupFailed, "invalid function metadata needs lookup failure");
        RequireNameFailureDiagnostic(result);
        Require(fixture.dispatchCalls == 0, "invalid function conversion cannot dispatch");
        Require(Read<std::uint32_t>(fixture.spawn.address(), 0xB0) == fixture.flags, "invalid function conversion must preserve flags");
    });
    tests.emplace_back("invalid InputSettings metadata conversion leaves console enabled", [] {
        Fixture fixture;
        fixture.faultyName = InputName;
        fixture.nameFault = NameFault::ChangedCapacity;
        ExpectKey(fixture, ConsoleKeyStatus::NameUnavailable);
        RequireNameFailureDiagnostic(Enable());
        Require(fixture.dispatchCalls == 1, "input metadata conversion failure must cache console success");
    });
    tests.emplace_back("unrelated failed metadata names do not hide valid lookups", [] {
        Fixture fixture;
        fixture.faultyName = ClassName;
        fixture.nameFault = NameFault::ChangedCapacity;
        ExpectKey(fixture, ConsoleKeyStatus::Available, "Tilde");
        Require(fixture.seenFunction == fixture.spawn.address(), "valid function must remain usable despite unrelated name failure");
    });
    tests.emplace_back("scratch contract violation does not poison later conversion", [] {
        Fixture fixture;
        fixture.nameFault = NameFault::ChangedData;
        ExpectKey(fixture, ConsoleKeyStatus::NameUnavailable);
        fixture.nameFault = NameFault::None;
        fixture.Start();
        const auto result = Enable();
        Require(result.status == ConsoleStatus::Enabled && result.keyStatus == ConsoleKeyStatus::Available,
                "later healthy conversion must recover after contract violation");
        Require(result.keyName == "Tilde", "later healthy conversion must report its own text");
        Require(fixture.cleanScratch && fixture.stableScratch, "borrowed replacement must not change fix-owned scratch storage");
        Require(fixture.dispatchCalls == 2, "explicit reinitialization permits one fresh console spawn");
    });
    return tests;
}
} // namespace

int main()
{
    const auto tests = Tests();
    std::size_t passed = 0;
    for (const auto& [name, test] : tests)
    {
        try
        {
            test();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        }
        catch (const std::exception& error)
        {
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
        catch (...)
        {
            std::cerr << "[FAIL] " << name << ": unknown exception\n";
        }
    }
    std::cout << passed << '/' << tests.size() << " checks passed\n";
    return passed == tests.size() ? 0 : 1;
}
