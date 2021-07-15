#pragma once
#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <array>
#include <memory>


#include "GbCpu.h"
#include "Devices/GbCartridge.h"
#include "GbClock.h"


class Application;

namespace Emulator {
    class GbCpu;
	class GbBus
	{
	public:
        GbBus();
        GbBus(bool stopClock);

		~GbBus();

		void SetOnRefreshUI(std::shared_ptr<std::function<void()>> func);

    private :
	    bool BootCompleted = false;

        std::shared_ptr<std::function<void()>> OnRefreshUI = nullptr;

    public: //devices
	    GbClock Clock {1024};

		std::unique_ptr<Emulator::GbCpu> Cpu;

		std::array<uint8_t, 0x00FF> BootROM{};                       // 0x0000 -> 0x00FF

		std::unique_ptr<GbCartridge> Cartridge = nullptr;            // 0x0000 -> 0x7FFF
//        std::array<uint8_t, 0x3FFF> GameRom0;
//        std::array<uint8_t, 0x3FFF> GameRomX;

        std::array<uint8_t, 0x1FFF> VideoRam{};                      // 0x8000 -> 0x9FFF
        std::array<uint8_t, 0x1FFF> ExternalRam{};                   // 0xA000 -> 0xBFFF
        std::array<uint8_t, 0x1FFF> WorkRam{};                       // 0xC000 -> 0xDFFF
        std::array<uint8_t, 0x009F> SpriteAttributeTable{};          // 0xFE00 -> 0xFE9F

        std::array<uint8_t, 0x007F> IORegisters{};                   // 0xFF00 -> 0xFF7F
        std::array<uint8_t, 0x007F> HighRam{};                       // 0xFF80 -> 0xFFFE

        uint8_t                     InterruptEnableRegister = 0xFF;  // 0xFFFF

	public:
		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address, bool readonly);


	};
}
#endif // !BUS_H