#include "Bus.h"
namespace Emulator {
	Bus::Bus()
	{
		for (auto& item : ram) {
			item = 0x00;
		}

		Cpu.Connect(this);
	}

	void Bus::Write(uint16_t address, uint8_t data) {
		if (address >= 0x0000 && address <= 0xFFFF) {
			ram[address] = data;
		}
	}
	uint8_t Bus::Read(uint16_t address, bool readonly = false) {
		if (address >= 0x0000 && address <= 0xFFFF) {
			return ram[address];
		}

		return 0x00;
	}
}