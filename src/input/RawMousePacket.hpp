#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <windows.h>

namespace p3r::input
{
using RawInputReader = UINT(WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);

struct MouseDelta
{
    LONG x;
    LONG y;
};

enum class PacketStatus
{
    RelativeMouse,
    Ignored,
    Invalid,
    AllocationFailed
};

struct PacketResult
{
    PacketStatus status;
    MouseDelta delta;
};

// DWORD-aligned storage satisfies GetRawInputData's buffer alignment contract.
// Tests may supply a failing allocator without exhausting system memory.
using RawInputBuffer = std::unique_ptr<std::uint32_t[]>;
using RawInputAllocator = RawInputBuffer (*)(std::size_t wordCount) noexcept;

PacketResult ReadRelativeMousePacket(HRAWINPUT input, RawInputReader reader,
                                     RawInputAllocator allocator = nullptr) noexcept;

// The message hook calls this only for removed WM_INPUT messages. Keeping this
// small operation shared lets tests verify the existing camera interpretation.
void ApplyRelativeMousePacket(const PacketResult& packet, float& accumulatedX, float& accumulatedY,
                              bool& lastValidInputWasFromMouse) noexcept;
} // namespace p3r::input
