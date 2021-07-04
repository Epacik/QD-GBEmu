#include "Cpu.h"


namespace Emulator {
	Cpu::Cpu()
	{
	}

	Cpu::~Cpu()
	{
	}

	void Cpu::Connect(Emulator::Bus* bus) {
        this->bus = bus;
	}

	void Cpu::Write(uint16_t address, uint8_t data)
	{
	    if(bus != nullptr)
		    bus->Write(address, data);
	}

	uint8_t Cpu::Read(uint16_t address)
	{
	    if(bus != nullptr)
		    return bus->Read(address, false);
	    return 0x00;
	}
	
	void Cpu::SetFlag(Flags flag)
	{
	}
}