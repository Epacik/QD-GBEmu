#pragma once
#include <memory>
#include "GbBus.h"

namespace Emulator {
    class BusDevice {
    public:
        void Connect(std::shared_ptr<GbBus> bus) {
            Bus = bus;
        }
    protected:
        std::shared_ptr<GbBus> Bus;
    };
}