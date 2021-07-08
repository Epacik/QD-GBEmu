#pragma once
#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <array>
#include <memory>


#include "GbCpu.h"
#include "Devices/GbCartridge.h"

namespace Emulator {
    class GbCpu;
	class Bus
	{
	public:
		Bus();
		~Bus();

    private : bool BootCompleted = false;
    public: //devices
		std::unique_ptr<Emulator::GbCpu> Cpu;

		std::array<uint8_t, 0x00FF> BootROM;

		std::unique_ptr<GbCartridge> Cartridge = nullptr;
//        std::array<uint8_t, 0x3FFF> GameRom0;
//        std::array<uint8_t, 0x3FFF> GameRomX;

        std::array<uint8_t, 0x1FFF> VideoRam;
        std::array<uint8_t, 0x1FFF> ExternalRam;
        std::array<uint8_t, 0x1FFF> WorkRam;
        std::array<uint8_t, 0x009F> SpriteAttributeTable;

        std::array<uint8_t, 0x007F> IORegisters;
        std::array<uint8_t, 0x007F> HighRam;

        uint8_t                     InterruptEnableRegister = 0x00;

	public:
		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address, bool readonly);
	};
}
#endif // !BUS_H