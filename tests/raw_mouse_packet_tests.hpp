#pragma once

#include "input/RawMousePacket.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>

namespace raw_mouse_tests
{
using namespace p3r::input;

// These literal offsets are the independently reviewed Windows x64 ABI:
// RAWINPUTHEADER is 24 bytes, followed by a 24-byte RAWMOUSE payload.
constexpr UINT headerSize = 0x18;
constexpr UINT mouseSize = 0x30;
constexpr UINT readError = static_cast<UINT>(-1);

struct ReaderFixture
{
    std::array<std::byte, mouseSize> bytes{};
    UINT queryResult = 0;
    UINT capacity = mouseSize;
    UINT copiedCount = mouseSize;
    UINT returnedSize = mouseSize;
    int calls = 0;
    bool argumentsValid = true;

    template <typename T>
    void Write(std::size_t offset, const T& value)
    {
        std::memcpy(bytes.data() + offset, &value, sizeof(value));
    }

    ReaderFixture(LONG x = 1200, LONG y = -2400)
    {
        Write(0x00, static_cast<DWORD>(RIM_TYPEMOUSE));
        Write(0x04, static_cast<DWORD>(mouseSize));
        Write(0x18, static_cast<USHORT>(0));
        Write(0x24, x);
        Write(0x28, y);
    }
};

inline ReaderFixture* activeFixture = nullptr;

HRAWINPUT FixtureHandle()
{
    return reinterpret_cast<HRAWINPUT>(static_cast<std::uintptr_t>(0x1234));
}

UINT WINAPI FixtureReader(HRAWINPUT input, UINT command, LPVOID data, PUINT size, UINT sizeHeader)
{
    auto& fixture = *activeFixture;
    ++fixture.calls;
    fixture.argumentsValid &=
        input == FixtureHandle() && command == RID_INPUT && size != nullptr && sizeHeader == headerSize;
    if (data == nullptr)
    {
        fixture.argumentsValid &= fixture.calls == 1 && *size == 0;
        *size = fixture.capacity;
        return fixture.queryResult;
    }

    fixture.argumentsValid &=
        fixture.calls == 2 && *size == fixture.capacity && reinterpret_cast<std::uintptr_t>(data) % 4 == 0;
    const std::size_t available = std::min<std::size_t>(fixture.capacity, fixture.bytes.size());
    const std::size_t written =
        fixture.copiedCount == readError ? 0 : std::min<std::size_t>(available, fixture.copiedCount);
    std::memcpy(data, fixture.bytes.data(), written);
    *size = fixture.returnedSize;
    return fixture.copiedCount;
}

PacketResult Read(ReaderFixture& fixture, RawInputAllocator allocator = nullptr)
{
    activeFixture = &fixture;
    const auto result = ReadRelativeMousePacket(FixtureHandle(), FixtureReader, allocator);
    activeFixture = nullptr;
    Require(fixture.argumentsValid, "reader receives the correct two-call Windows contract");
    return result;
}

void RequireNoMovement(const PacketResult& packet, PacketStatus expected)
{
    Require(packet.status == expected, "packet has expected failure or ignored status");
    Require(packet.delta.x == 0 && packet.delta.y == 0, "unaccepted packet has no delta");
    for (const bool initialMouseActivity : {false, true})
    {
        float x = 2.0f;
        float y = -3.0f;
        bool mouseActivity = initialMouseActivity;
        ApplyRelativeMousePacket(packet, x, y, mouseActivity);
        Require(x == 2.0f && y == -3.0f, "unaccepted packet leaves accumulated motion unchanged");
        Require(mouseActivity == initialMouseActivity, "unaccepted packet preserves input ownership");
    }
}

RawInputBuffer FailAllocation(std::size_t) noexcept
{
    return {};
}
} // namespace raw_mouse_tests

void RunRawMousePacketTests(const std::function<void(const char*, const std::function<void()>&)>& runTest)
{
    using namespace raw_mouse_tests;

    runTest("successful two-call mouse read preserves signed movement and sensitivity", [] {
        ReaderFixture fixture;
        const auto packet = Read(fixture);
        Require(fixture.calls == 2, "one query and one read");
        Require(packet.status == PacketStatus::RelativeMouse, "valid relative mouse packet");
        Require(packet.delta.x == 1200 && packet.delta.y == -2400, "signed deltas preserved");
        float x = 0.5f;
        float y = 0.25f;
        bool mouseActivity = false;
        ApplyRelativeMousePacket(packet, x, y, mouseActivity);
        Require(x == 1.5f && y == -1.75f, "valid delta is accumulated once using the existing divisor");
        Require(mouseActivity, "nonzero relative motion activates the mouse");
    });

    runTest("zero motion preserves existing input ownership", [] {
        ReaderFixture fixture(0, 0);
        const auto packet = Read(fixture);
        Require(packet.status == PacketStatus::RelativeMouse, "zero-motion packet remains valid");
        for (const bool initialMouseActivity : {false, true})
        {
            float x = 1.0f;
            float y = -1.0f;
            bool mouseActivity = initialMouseActivity;
            ApplyRelativeMousePacket(packet, x, y, mouseActivity);
            Require(x == 1.0f && y == -1.0f, "zero motion preserves accumulators");
            Require(mouseActivity == initialMouseActivity, "zero motion preserves active device");
        }
    });

    runTest("extreme signed deltas retain the existing camera interpretation", [] {
        ReaderFixture fixture((std::numeric_limits<LONG>::min)(), (std::numeric_limits<LONG>::max)());
        const auto packet = Read(fixture);
        float x = 0.0f;
        float y = 0.0f;
        bool mouseActivity = false;
        ApplyRelativeMousePacket(packet, x, y, mouseActivity);
        Require(std::isfinite(x) && std::isfinite(y) && x < 0.0f && y > 0.0f,
                "extreme signed motion produces finite deltas with the correct signs");
        Require(mouseActivity, "extreme motion activates the mouse");
    });

    runTest("invalid reader or input handle is rejected without a read", [] {
        RequireNoMovement(ReadRelativeMousePacket(FixtureHandle(), nullptr), PacketStatus::Invalid);
        ReaderFixture fixture;
        activeFixture = &fixture;
        const auto result = ReadRelativeMousePacket(nullptr, FixtureReader);
        activeFixture = nullptr;
        RequireNoMovement(result, PacketStatus::Invalid);
        Require(fixture.calls == 0, "invalid handle never reaches the reader");
    });

    runTest("size-query errors and unexpected return values stop before allocation", [] {
        for (const UINT result : {readError, 1u})
        {
            ReaderFixture fixture;
            fixture.queryResult = result;
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
            Require(fixture.calls == 1, "failed query has no data read");
        }
    });

    runTest("zero and undersized queried capacities are rejected", [] {
        for (const UINT capacity : {0u, headerSize - 1})
        {
            ReaderFixture fixture;
            fixture.capacity = capacity;
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
            Require(fixture.calls == 1, "undersized query has no data read");
        }
    });

    runTest("allocation failure leaves movement and ownership unchanged", [] {
        ReaderFixture fixture;
        RequireNoMovement(Read(fixture, FailAllocation), PacketStatus::AllocationFailed);
        Require(fixture.calls == 1, "allocation failure prevents the data read");
    });

    runTest("failed and short data reads are rejected before header parsing", [] {
        for (const UINT copied : {readError, 0u, headerSize - 1, headerSize, mouseSize - 1})
        {
            ReaderFixture fixture;
            fixture.copiedCount = copied;
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
            Require(fixture.calls == 2, "bad data read still uses only the original two calls");
        }
    });

    runTest("an oversized returned byte count is rejected", [] {
        ReaderFixture fixture;
        fixture.copiedCount = mouseSize + 1;
        RequireNoMovement(Read(fixture), PacketStatus::Invalid);
    });

    runTest("the API cannot disguise a short read by changing the size argument", [] {
        ReaderFixture fixture;
        fixture.copiedCount = mouseSize - 1;
        fixture.returnedSize = mouseSize - 1;
        RequireNoMovement(Read(fixture), PacketStatus::Invalid);
    });

    runTest("inconsistent returned size arguments are rejected", [] {
        for (const UINT size : {0u, mouseSize - 1, mouseSize + 1})
        {
            ReaderFixture fixture;
            fixture.returnedSize = size;
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
        }
    });

    runTest("inconsistent declared packet sizes are rejected", [] {
        for (const DWORD size : {0ul, static_cast<DWORD>(headerSize - 1), static_cast<DWORD>(mouseSize - 1),
                                 static_cast<DWORD>(mouseSize + 1)})
        {
            ReaderFixture fixture;
            fixture.Write(0x04, size);
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
        }
    });

    runTest("a complete header cannot authorize a truncated mouse payload", [] {
        for (const UINT size : {headerSize, mouseSize - 1})
        {
            ReaderFixture fixture;
            fixture.capacity = size;
            fixture.copiedCount = size;
            fixture.returnedSize = size;
            fixture.Write(0x04, static_cast<DWORD>(size));
            RequireNoMovement(Read(fixture), PacketStatus::Invalid);
        }
    });

    runTest("valid non-mouse packets are ignored", [] {
        for (const DWORD type : {static_cast<DWORD>(RIM_TYPEKEYBOARD), static_cast<DWORD>(RIM_TYPEHID)})
        {
            ReaderFixture fixture;
            fixture.Write(0x00, type);
            RequireNoMovement(Read(fixture), PacketStatus::Ignored);
        }
    });

    runTest("nonzero mouse flags retain the baseline selection behavior", [] {
        for (const USHORT flags :
             {static_cast<USHORT>(MOUSE_MOVE_ABSOLUTE), static_cast<USHORT>(MOUSE_VIRTUAL_DESKTOP),
              static_cast<USHORT>(MOUSE_ATTRIBUTES_CHANGED), static_cast<USHORT>(MOUSE_MOVE_NOCOALESCE)})
        {
            ReaderFixture fixture;
            fixture.Write(0x18, flags);
            RequireNoMovement(Read(fixture), PacketStatus::Ignored);
        }
    });
}
