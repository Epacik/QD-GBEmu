//
// Created by epat on 11.08.2021.
//

#ifndef GBEMU_INTERRUPTS_H
#define GBEMU_INTERRUPTS_H

namespace Emulator{
    enum class Interrupts {
        VerticalBlankInterrupt = 1,
        LcdStatInterrupt       = 1 << 1,
        TimerInterrupt         = 1 << 2,
        SerialInterrupt        = 1 << 3,
        JoypadInterrupt        = 1 << 4,
        };
}

#endif //GBEMU_INTERRUPTS_H
