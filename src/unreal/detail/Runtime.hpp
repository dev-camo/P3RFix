#pragma once

#include "../Integration.hpp"

#include <cstdint>

namespace p3r::unreal::detail {

InitStatus InitializeRuntime(RuntimeAddresses addresses) noexcept;
ConsoleResult TryEnableConsole();

// Borrowed registry lookup, exposed only to test chunk traversal and validation.
// It returns null for an unavailable/invalid registry, invalid index or hole.
void* ObjectAtIndex(std::int32_t index) noexcept;

} // namespace p3r::unreal::detail
