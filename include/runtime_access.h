#pragma once

#include <ranges>

template<typename ... Devices>
class RuntimeAccess {
    public:
        RuntimeAccess(std::ranges::range auto &deviceInfo);
    private:
};