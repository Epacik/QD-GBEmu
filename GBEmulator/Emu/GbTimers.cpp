//
// Created by epat on 29.07.2021.
//

#include "GbTimers.h"
namespace Emulator {
    uint8_t GbTimers::GetDividerRegister() const {
        return (uint8_t)((MainRegister & 0xFF00) >> 8);
    }

    void GbTimers::OnClockCycle() {
        uint16_t v1 = MainRegister & (1 << speeds[TimerControl & 0b11]);
        MainRegister++;

        if((TimerControl & 0b100) == 0)
        {
            if (v1 > 0) TimerCounter++;
            return;
        }
        uint16_t v2 = MainRegister & (1 << speeds[TimerControl & 0b11]);

        if(v1 > 0 && v2 == 0)
            TimerCounter++;

    }

    void GbTimers::Write(uint16_t address, uint8_t data) {
        switch(address - 0xFF04){
            case 0: MainRegister = 0;
            case 1: TimerCounter = data;
            case 2: TimerModulo  = data;
            case 3: TimerControl = data & 0b00000111;
        }
    }

    uint8_t GbTimers::Read(uint16_t address) const {
        switch(address - 0xFF04){
            case 0:  return GetDividerRegister();
            case 1:  return TimerCounter;
            case 2:  return TimerModulo;
            case 3:  return TimerControl & 0b00000111;
            default: return 0;
        }
    }
}