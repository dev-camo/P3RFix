#include "RawMousePacket.hpp"

#include <cstring>
#include <limits>
#include <new>

namespace p3r::input
{
namespace
{
constexpr UINT readError = static_cast<UINT>(-1);

RawInputBuffer AllocateBuffer(std::size_t wordCount) noexcept
{
    return RawInputBuffer(new (std::nothrow) std::uint32_t[wordCount]);
}

PacketResult NoMovement(PacketStatus status) noexcept
{
    return {status, {0, 0}};
}
} // namespace

PacketResult ReadRelativeMousePacket(HRAWINPUT input, RawInputReader reader,
                                     RawInputAllocator allocator) noexcept
{
    if (input == nullptr || reader == nullptr)
        return NoMovement(PacketStatus::Invalid);

    UINT capacity = 0;
    const UINT queryResult = reader(input, RID_INPUT, nullptr, &capacity, sizeof(RAWINPUTHEADER));
    if (queryResult != 0 || capacity < sizeof(RAWINPUTHEADER))
        return NoMovement(PacketStatus::Invalid);

    // Round up for DWORD alignment without allowing the allocation arithmetic
    // to overflow. The API still receives the original capacity in bytes.
    constexpr std::size_t wordSize = sizeof(std::uint32_t);
    const std::size_t byteCapacity = capacity;
    if (byteCapacity > (std::numeric_limits<std::size_t>::max)() - (wordSize - 1))
        return NoMovement(PacketStatus::Invalid);

    const std::size_t wordCount = (byteCapacity + wordSize - 1) / wordSize;
    RawInputBuffer buffer = (allocator != nullptr ? allocator : &AllocateBuffer)(wordCount);
    if (!buffer)
        return NoMovement(PacketStatus::AllocationFailed);

    UINT readSize = capacity;
    const UINT copiedCount = reader(input, RID_INPUT, buffer.get(), &readSize, sizeof(RAWINPUTHEADER));
    if (copiedCount == readError || copiedCount != capacity || readSize != capacity)
        return NoMovement(PacketStatus::Invalid);

    // Copy into SDK objects after checking the available bytes. DWORD alignment
    // alone does not imply the stronger alignment of the x64 packet header.
    RAWINPUTHEADER header{};
    std::memcpy(&header, buffer.get(), sizeof(header));
    if (header.dwSize < sizeof(header) || header.dwSize != copiedCount)
        return NoMovement(PacketStatus::Invalid);

    if (header.dwType != RIM_TYPEMOUSE)
        return NoMovement(PacketStatus::Ignored);

    constexpr std::size_t mouseOffset = offsetof(RAWINPUT, data);
    constexpr std::size_t mousePacketSize = mouseOffset + sizeof(RAWMOUSE);
    if (copiedCount < mousePacketSize)
        return NoMovement(PacketStatus::Invalid);

    RAWMOUSE mouse{};
    const auto* bytes = reinterpret_cast<const std::byte*>(buffer.get());
    std::memcpy(&mouse, bytes + mouseOffset, sizeof(mouse));

    // Retain the current fix's exact flag selection. Other relative flag
    // combinations need a separate camera-behavior change and validation.
    if (mouse.usFlags != 0)
        return NoMovement(PacketStatus::Ignored);

    return {PacketStatus::RelativeMouse, {mouse.lLastX, mouse.lLastY}};
}

void ApplyRelativeMousePacket(const PacketResult& packet, float& accumulatedX, float& accumulatedY,
                              bool& lastValidInputWasFromMouse) noexcept
{
    if (packet.status != PacketStatus::RelativeMouse)
        return;

    if (packet.delta.x != 0 || packet.delta.y != 0)
        lastValidInputWasFromMouse = true;

    // Preserve the existing mouse fix's sensitivity and signed movement.
    accumulatedX += static_cast<float>(packet.delta.x) / 1200.0f;
    accumulatedY += static_cast<float>(packet.delta.y) / 1200.0f;
}
} // namespace p3r::input
