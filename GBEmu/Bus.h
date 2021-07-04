#pragma once
#include <cstdint>
#include "Cpu.h"
#include <array>
namespace Emulator {


	class Bus
	{
	public:
		Bus();
		~Bus();

	public: //devices
		Emulator::Cpu Cpu;

		std::array<uint8_t, 64 * 1024> ram;
	public:
		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address, bool readonly = false);
	};
}
