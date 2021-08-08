//
// Created by epat on 29.07.2021.
//

#include "GbTimers.h"
namespace Emulator {
    uint8_t GbTimers::GetDividerRegister() const {
        return (uint8_t)((MainRegister & 0xFF00) >> 8);
    }

    uint16_t mainRegisterOldValue = 0x0000;
    void GbTimers::OnClockCycle() {
        if (TimerOverflowedOn + 5 == MainRegister)
        {
            HandleTimerCounterOverflow();
        }
        
        MainRegister++;
       
        if (GetSpeedBitValue(mainRegisterOldValue) > 0) {
            TimerIncrement();
        }

        mainRegisterOldValue = MainRegister;

    }

    void GbTimers::TimerIncrement()
    {
        if (!IsTimerEnabled())
        {
            TimerCounter++;
            return;
        }
        
        if (GetSpeedBitValue(MainRegister) == 0 && TimerCounter++ == 0)
        {
            TimerOverflowedOn = MainRegister;
        }
    }

    bool GbTimers::IsTimerEnabled()
    {
        return (TimerControl & 0b100) != 0;
    }

    int GbTimers::GetSpeedBitValue(uint16_t value)
    {
        return value & (1 << SpeedBits[TimerControl & 0b11]);
    }
    
    void GbTimers::HandleTimerCounterOverflow()
    {
        Bus->Cpu->SetInterruptFlag(GbCpu::Interrupts::TimerInterrupt);

        if(!PreventCopyingTimerModuloToTimerCounter)
            TimerCounter = TimerModulo;
        PreventCopyingTimerModuloToTimerCounter = false;
    }

    bool canSetInterrupt = false;

    void GbTimers::Write(uint16_t address, uint8_t data) {
        switch(address - 0xFF04){
            case 0:
                MainRegister = 0;
                break;
            case 1:
                TimerCounter = data;
                if (TimerOverflowedOn + 4 >= MainRegister)
                    PreventCopyingTimerModuloToTimerCounter = true;
                canSetInterrupt = false;
                break;
            case 2:
                TimerModulo = data; 
                break;
            case 3:
                auto value = data & 0b00000111;
                if (TimerControl & 0b11 == 0 && value == 0b101)
                    TimerCounter++;
                TimerControl = value;

                break;
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