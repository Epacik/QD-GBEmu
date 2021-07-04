#include "Bus.h"

namespace Emulator {
	Bus::Bus()
	{
		for (auto& item : Ram) {
			item = 0x00;
		}

		cpu.Connect(this);
	}

	Bus::~Bus()
	{
	}

	void Bus::Write(uint16_t address, uint8_t data) {
		if (address >= 0x0000 && address <= 0xFFFF) {
			Ram[address] = data;
		}
	}
	uint8_t Bus::Read(uint16_t address, bool readonly) {
		if (address >= 0x0000 && address <= 0xFFFF) {
			return Ram[address];
		}

		return 0x00;
	}
}