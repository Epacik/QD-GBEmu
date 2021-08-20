#pragma once
#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <array>
#include <memory>
#include <iostream>

#include "Interrupts.h"
#include "GbCpu.h"
#include "Devices/GbCartridge.h"
#include "Devices/Cartridges.h"
#include "GbClock.h"
#include "GbTimers.h"
#include "GbJoypad.h"


class Application;

namespace Emulator {
    class GbCpu;
    class GbCartridgeBase;
    class GbJoypad;
    class GbTimers;

    class GbBus : public std::enable_shared_from_this<GbBus>
    {
    public:
        GbBus();
        void ConnectDevices();
        void InitializeMemory();
        void SetOnClockCycle();
        explicit GbBus(bool stopClock);

        ~GbBus();

        void SetOnRefreshUI(std::shared_ptr<std::function<void()>> func);

    private :
        bool BootCompleted = false;

        std::shared_ptr<std::function<void()>> OnRefreshUI = nullptr;

        uint16_t clockCycle = 0;

    public: //devices
        GbClock Clock {4};

        void HangTheWholeEmulator();
        std::unique_ptr<Emulator::GbCpu>    Cpu;
        

        std::array<uint8_t, 0x00FF> BootROM{};                       // 0x0000 -> 0x00FF

        std::unique_ptr<Emulator::Cartridges::GbCartridgeBase> Cartridge;
                                                                    // 0x0000 -> 0x7FFF 
                                                                    // & 0xA000 -> 0xBFFF


        std::array<uint8_t, 0x1FFF> VideoRam{};                      // 0x8000 -> 0x9FFF
        //std::array<uint8_t, 0x1FFF> ExternalRam{};                 //
        std::array<uint8_t, 0x1FFF> WorkRam{};                       // 0xC000 -> 0xDFFF
        std::array<uint8_t, 0x009F> SpriteAttributeTable{};          // 0xFE00 -> 0xFE9F

        std::unique_ptr <Emulator::GbJoypad> Joypad;                 // 0xFF00
        std::unique_ptr <Emulator::GbTimers> Timers;                 // 0xFF04 -> 0xFF07
        std::array<uint8_t, 0x007F> IORegisters{};                   // 0xFF00 -> 0xFF7F
        std::array<uint8_t, 0x007F> HighRam{};                       // 0xFF80 -> 0xFFFE

        uint8_t                     InterruptEnableRegister = 0xFF;  // 0xFFFF

    public:
        void Write(uint16_t address, uint8_t data);
        uint8_t Read(uint16_t address, bool readonly);
        uint8_t Read(uint16_t address){
            return Read(address, false);
        }


        bool TurnLcdOff;

        void SetInterruptFlag(Emulator::Interrupts interrupt);
    };
}
#endif // !BUS_H