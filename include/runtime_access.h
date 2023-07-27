#pragma once

#include <ranges>

// This class is not allowed to use chip specific class or headers, it should be consumed by other devices, 
// which want to communicate with this i2c_slave

template<typename ... Devices>
class RuntimeAccess {
    public:
        RuntimeAccess(std::ranges::range auto &deviceInfo);
    private:
};