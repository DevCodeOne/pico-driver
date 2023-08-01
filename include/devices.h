#pragma once

#include <cstdint>
#include <type_traits>

namespace PicoDriver {
    template<typename ... Devices>
    struct DeviceList final : public std::tuple<Devices ...> {
        template<typename Device>
        using AppendDevice = DeviceList<Device, Devices ...>;
    };

}