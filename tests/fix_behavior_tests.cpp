// Test the shipped helpers with independent arithmetic answers and Windows packets.
#define NOMINMAX

#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace
{
void Require(bool condition, const char* explanation)
{
    if (!condition)
        throw std::runtime_error(explanation);
}
} // namespace

#include "raw_mouse_packet_tests.hpp"
#include "render_scaling_tests.hpp"

int main()
{
    int passed = 0;
    int total = 0;
    const auto runTest = [&](const char* name, const std::function<void()>& test) {
        ++total;
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
    };

    RunRenderScalingTests(runTest);
    RunRawMousePacketTests(runTest);
    std::cout << passed << '/' << total << " checks passed\n";
    return passed == total ? 0 : 1;
}
