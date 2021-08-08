//
// Created by epat on 08.08.2021.
//

#ifndef GBEMU_GBJOYPAD_H
#define GBEMU_GBJOYPAD_H
#include "../global.h"
#include "BusDevice.h"

namespace Emulator {

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

    class GbJoypad : public BusDevice {
    private:
        GbJoypadButtons ButtonsPressed = (GbJoypadButtons)0;

        uint8_t selectedMap = 0x00;
    public:
        uint8_t Read();

        void Write(uint8_t value);

        void PressButtons(GbJoypadButtons buttons);

    };
}


#endif //GBEMU_GBJOYPAD_H
