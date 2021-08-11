#pragma once
#include "GbBus.h"

namespace Emulator {
    class GbBus;
    class BusDevice {
    public:
        virtual void Connect(GbBus* bus) = 0;
    protected:
        GbBus *Bus;
    };
}