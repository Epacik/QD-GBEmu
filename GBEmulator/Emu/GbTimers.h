//
// Created by epat on 29.07.2021.
//
#ifndef GBEMU_GBTIMERS_H
#define GBEMU_GBTIMERS_H

#include <memory>
#include <array>
#include <cstdint>
#include <array>
#include "Interrupts.h"
#include "GbBus.h"
#include "GbCpu.h"

namespace Emulator{
    class GbBus;
    class GbTimers {
    private:
        uint16_t MainRegister = 0xABCC;

        uint8_t TimerCounter = 0x00;
        uint8_t TimerModulo  = 0x00;
        uint8_t TimerControl = 0x00;

        uint16_t TimerOverflowedOn = 0x00 - 5;
        bool PreventCopyingTimerModuloToTimerCounter = false;

        std::array<uint16_t, 4> SpeedBits = { 9, 3, 5, 7 };

        int GetSpeedBitValue(uint16_t value);
        void HandleTimerCounterOverflow();

    public:
        void OnClockCycle();

        void TimerIncrement();

        bool IsTimerEnabled();
        
        uint8_t GetDividerRegister() const;
        void    Write(uint16_t address, uint8_t data);
        uint8_t Read(uint16_t address) const;

    public:
        void Connect(GbBus* bus);
    protected:
        GbBus* Bus;

    };
}


#endif //GBEMU_GBTIMERS_H
