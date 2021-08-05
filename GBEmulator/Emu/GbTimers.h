//
// Created by epat on 29.07.2021.
//

#include "../global.h"

#ifndef GBEMU_GBTIMERS_H
#define GBEMU_GBTIMERS_H

namespace Emulator{
    class GbTimers {
    private:
        uint16_t MainRegister = 0xABCC;

        uint8_t TimerCounter = 0x00;
        uint8_t TimerModulo  = 0x00;
        uint8_t TimerControl = 0x00;

        std::array<uint16_t, 4> speeds { 9, 3, 5, 7 };

    public:
        void    OnClockCycle();
        uint8_t GetDividerRegister() const;
        void    Write(uint16_t address, uint8_t data);
        uint8_t Read(uint16_t address) const;

    };
}


#endif //GBEMU_GBTIMERS_H
