#pragma once

/*
 * Focused memory descriptions extracted from the Persona 3 Reload SDK generated
 * by Dumper-7: https://github.com/Encryqed/Dumper-7. Original generated symbols
 * are named beside each record. These describe borrowed game memory, not objects
 * constructed by this fix. The baseline dump is retained in Git history.
 */

#include "Containers.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace p3r::unreal::detail {

// SDK::FName in Basic.hpp; Number is passed intact to AppendString, which formats
// numbered names. No fix-owned name table or suffix formatter is necessary.
struct Name {
    std::int32_t comparisonIndex;
    std::uint32_t number;
};

// SDK::UObject in CoreUObject_classes.hpp.
struct Object {
    void* vtable;
    std::int32_t flags;
    std::int32_t index;
    void* objectClass;
    Name name;
    void* outer;
};

// SDK::UField, SDK::UStruct, SDK::UClass and SDK::UFunction in
// CoreUObject_classes.hpp. Explicit flat padding replaces C++ inheritance.
struct Field {
    std::byte padding0[0x28];
    void* next;
};
struct Struct {
    std::byte padding0[0x40];
    void* super;
    void* children;
    std::byte padding50[0x60];
};
struct Class {
    std::byte padding0[0x40];
    void* super;
    void* children;
    std::byte padding50[0x80];
    std::uint64_t castFlags;
    std::byte paddingD8[0x40];
    void* defaultObject;
    std::byte padding120[0x110];
};
struct alignas(8) Function {
    std::byte padding0[0xB0];
    std::uint32_t functionFlags;
    std::byte paddingB4[0x2C];
};

// SDK::FUObjectItem and SDK::TUObjectArray in Basic.hpp. DecryptPtr in the
// inherited implementation is identity; chunk pointers need no transformation.
struct ObjectItem {
    void* object;
    std::byte padding8[0x10];
};
struct ObjectArray {
    void* chunks;
    std::byte padding8[8];
    std::int32_t maxElements;
    std::int32_t numElements;
    std::int32_t maxChunks;
    std::int32_t numChunks;
};

// SDK::FKey in InputCore_structs.hpp.
struct alignas(8) Key {
    Name name;
    std::byte padding8[0x10];
};

// SDK::Params::GameplayStatics_SpawnObject in Engine_parameters.hpp.
struct SpawnObjectParameters {
    void* objectClass;
    void* outer;
    void* returnValue;
};

// Prefix views of SDK::UEngine, SDK::UGameViewportClient and
// SDK::UInputSettings in Engine_classes.hpp; sizes below are only view sizes.
struct EngineView {
    std::byte padding0[0xF0];
    void* consoleClass;
    std::byte paddingF8[0x688];
    void* gameViewport;
};
struct ViewportView {
    std::byte padding0[0x40];
    void* viewportConsole;
};
struct InputSettingsView {
    std::byte padding0[0x130];
    ArrayHeader consoleKeys;
};

// Prefix of SDK::UTextureRenderTarget2D in Engine_classes.hpp. The generated
// type is packed and aligned to 16; explicit bytes preserve the used fields.
struct alignas(16) RenderTargetView {
    std::byte padding0[0x180];
    std::int32_t sizeX;
    std::int32_t sizeY;
    std::byte padding188[0x13];
    std::uint8_t renderTargetFormat;
};

// Capture offsets previously written by RTCaptureMidHook in dllmain.cpp.
struct CaptureView {
    std::byte padding0[0x1FC];
    std::int32_t width;
    std::int32_t height;
};

#define P3R_LAYOUT(Type, Size, Alignment) \
    static_assert(std::is_standard_layout_v<Type>); \
    static_assert(sizeof(Type) == Size); \
    static_assert(alignof(Type) == Alignment)
#define P3R_FIELD(Type, Member, Offset) static_assert(offsetof(Type, Member) == Offset)

P3R_LAYOUT(Name, 8, 4);
P3R_FIELD(Name, comparisonIndex, 0);
P3R_FIELD(Name, number, 4);
P3R_LAYOUT(Object, 0x28, 8);
P3R_FIELD(Object, vtable, 0);
P3R_FIELD(Object, flags, 8);
P3R_FIELD(Object, index, 0xC);
P3R_FIELD(Object, objectClass, 0x10);
P3R_FIELD(Object, name, 0x18);
P3R_FIELD(Object, outer, 0x20);
P3R_LAYOUT(Field, 0x30, 8);
P3R_FIELD(Field, next, 0x28);
P3R_LAYOUT(Struct, 0xB0, 8);
P3R_FIELD(Struct, super, 0x40);
P3R_FIELD(Struct, children, 0x48);
P3R_LAYOUT(Class, 0x230, 8);
P3R_FIELD(Class, super, 0x40);
P3R_FIELD(Class, children, 0x48);
P3R_FIELD(Class, castFlags, 0xD0);
P3R_FIELD(Class, defaultObject, 0x118);
P3R_LAYOUT(Function, 0xE0, 8);
P3R_FIELD(Function, functionFlags, 0xB0);
P3R_LAYOUT(ObjectItem, 0x18, 8);
P3R_FIELD(ObjectItem, object, 0);
P3R_LAYOUT(ObjectArray, 0x20, 8);
P3R_FIELD(ObjectArray, chunks, 0);
P3R_FIELD(ObjectArray, maxElements, 0x10);
P3R_FIELD(ObjectArray, numElements, 0x14);
P3R_FIELD(ObjectArray, maxChunks, 0x18);
P3R_FIELD(ObjectArray, numChunks, 0x1C);
P3R_LAYOUT(Key, 0x18, 8);
P3R_FIELD(Key, name, 0);
P3R_LAYOUT(SpawnObjectParameters, 0x18, 8);
P3R_FIELD(SpawnObjectParameters, objectClass, 0);
P3R_FIELD(SpawnObjectParameters, outer, 8);
P3R_FIELD(SpawnObjectParameters, returnValue, 16);
P3R_LAYOUT(EngineView, 0x788, 8);
P3R_FIELD(EngineView, consoleClass, 0xF0);
P3R_FIELD(EngineView, gameViewport, 0x780);
P3R_LAYOUT(ViewportView, 0x48, 8);
P3R_FIELD(ViewportView, viewportConsole, 0x40);
P3R_LAYOUT(InputSettingsView, 0x140, 8);
P3R_FIELD(InputSettingsView, consoleKeys, 0x130);
P3R_LAYOUT(RenderTargetView, 0x1A0, 16);
P3R_FIELD(RenderTargetView, sizeX, 0x180);
P3R_FIELD(RenderTargetView, sizeY, 0x184);
P3R_FIELD(RenderTargetView, renderTargetFormat, 0x19B);
P3R_LAYOUT(CaptureView, 0x204, 4);
P3R_FIELD(CaptureView, width, 0x1FC);
P3R_FIELD(CaptureView, height, 0x200);

#undef P3R_LAYOUT
#undef P3R_FIELD

// SDK::TUObjectArray::ElementsPerChunk and SDK::Offsets::ProcessEventIdx
// in Basic.hpp; remaining object/cast masks from Basic.hpp and the native
// flag in UGameplayStatics::SpawnObject. Render format is
// SDK::ETextureRenderTargetFormat in Engine_structs.hpp.
inline constexpr std::int32_t ElementsPerChunk = 0x10000;
inline constexpr std::uint32_t ClassDefaultObjectFlag = 0x10;
inline constexpr std::uint64_t ClassCastFlag = 0x20;
inline constexpr std::uint64_t FunctionCastFlag = 0x80000;
inline constexpr std::uint32_t NativeFunctionFlag = 0x400;
inline constexpr std::size_t ProcessEventIndex = 0x44;
inline constexpr std::uint8_t Rgba16fFormat = 6;

} // namespace p3r::unreal::detail
