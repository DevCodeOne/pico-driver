#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {
    namespace DeviceTags {
        struct DRV8825 final : public std::integral_constant<DeviceId, DeviceId::DRV8825> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::DRV8825> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
        bool direction;
    } __attribute__((packed));


}