//
// Created by epat on 08.08.2021.
//

#ifndef GBEMU_GBJOYPAD_H
#define GBEMU_GBJOYPAD_H

#include <cstdint>
#include "Interrupts.h"
#include "GbBus.h"
#include "GbCpu.h"

namespace Emulator {
    class GbBus;
    class GbCpu;

    enum GbJoypadButtons {
        Start  = 1,
        Select = 1 << 1,
        B      = 1 << 2,
        A      = 1 << 3,
        Down   = 1 << 4,
        Up     = 1 << 5,
        Left   = 1 << 6,
        Right  = 1 << 7,
    };
    
    class BusDevice;
    class GbJoypad{
    private:
        GbJoypadButtons ButtonsPressed = (GbJoypadButtons)0;

        uint8_t selectedMap = 0x00;

        bool IsButtonPressed(GbJoypadButtons button);
    public:
        uint8_t Read();

        

        void Write(uint8_t value);

        void PressButtons(GbJoypadButtons buttons);


    public:
        void Connect(GbBus* bus);
    protected:
        GbBus* Bus;

    };
}


#endif //GBEMU_GBJOYPAD_H
