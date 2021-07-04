#pragma once
#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include <array>
#include <memory>


#include "Cpu.h"


namespace Emulator {
    class Cpu;

	class Bus
	{
	public:
		Bus();
		~Bus();

	public: //devices
		std::unique_ptr<Emulator::Cpu> cpu;

		std::array<uint8_t, 64 * 1024> Ram;
	public:
		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address, bool readonly);
	};
}
#endif // !BUS_H