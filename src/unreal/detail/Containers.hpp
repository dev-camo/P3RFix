#pragma once

/*
 * Array/string layout and scratch-buffer contract extracted from the SDK generated
 * by Dumper-7 (https://github.com/Encryqed/Dumper-7), which attributes its container
 * implementations to https://github.com/Fischsalat/UnrealContainers.
 * Only borrowed array reads and fix-owned name storage are retained here.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>

namespace p3r::unreal::detail {

static_assert(sizeof(void*) == 8, "The game integration requires the Windows x64 ABI");
static_assert(sizeof(wchar_t) == 2, "The game name callback requires 16-bit wchar_t");

// UC::TArray / UC::FString. Data is borrowed; this record never frees or grows it.
struct ArrayHeader {
    void* data;
    std::int32_t count;
    std::int32_t capacity;
};
static_assert(std::is_standard_layout_v<ArrayHeader>);
static_assert(sizeof(ArrayHeader) == 0x10);
static_assert(alignof(ArrayHeader) == 8);
static_assert(offsetof(ArrayHeader, data) == 0);
static_assert(offsetof(ArrayHeader, count) == 8);
static_assert(offsetof(ArrayHeader, capacity) == 12);

// Byte copies describe game memory without asserting that our C++ records live
// there. Callers must supply known valid game addresses; null guards do not make
// arbitrary non-null addresses safe.
template<class T>
T ReadMemory(const void* address, std::size_t offset) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    T value{};
    std::memcpy(&value, static_cast<const std::byte*>(address) + offset, sizeof(value));
    return value;
}

template<class T>
void WriteMemory(void* address, std::size_t offset, const T& value) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    std::memcpy(static_cast<std::byte*>(address) + offset, &value, sizeof(value));
}

inline bool IsValidArray(const ArrayHeader& array) noexcept {
    return array.count >= 0 && array.capacity >= array.count &&
           (array.count == 0 || array.data != nullptr);
}

// SDK::FName::GetRawString used UC::FAllocatedString(1024). Keep its
// no-reallocation contract and malloc/free allocator, but own the original
// allocation separately: a callback-modified header must never choose what we
// free. Every conversion restores the supplied header, including after failure.
class NameScratch final {
public:
    static constexpr std::int32_t Capacity = 1024;

    NameScratch() noexcept
        : storage_(static_cast<wchar_t*>(std::malloc(Capacity * sizeof(wchar_t))), &std::free) {
        Reset();
    }

    void Reset() noexcept {
        header = {storage_.get(), 0, Capacity};
    }

    bool HasValidResult() const noexcept {
        return storage_ && header.data == storage_.get() &&
               header.capacity == Capacity && header.count > 0 &&
               header.count <= Capacity && storage_.get()[header.count - 1] == L'\0';
    }

    const wchar_t* Data() const noexcept { return storage_.get(); }
    ArrayHeader header{};

private:
    std::unique_ptr<wchar_t, decltype(&std::free)> storage_;
};

} // namespace p3r::unreal::detail
