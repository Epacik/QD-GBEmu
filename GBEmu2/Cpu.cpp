#include "Cpu.h"

namespace Emulator {
	Cpu::Cpu()
	{
	}

	Cpu::~Cpu()
	{
	}



	void Cpu::Write(uint16_t address, uint8_t data)
	{
		Bus->Write(address, data);
	}

	uint8_t Cpu::Read(uint16_t address)
	{
		return Bus->Read(address, false);
	}
}