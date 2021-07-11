#include "GbCpu.h"


namespace Emulator {
	GbCpu::GbCpu()
	{
	}

	GbCpu::~GbCpu()
	{
	}

	void GbCpu::Connect(Emulator::Bus* bus) {
        this->bus = bus;
	}

	void GbCpu::Write(uint16_t address, uint8_t data)
	{
	    if(bus != nullptr)
		    bus->Write(address, data);
	}

	uint8_t GbCpu::Read(uint16_t address)
	{
	    if(bus != nullptr)
		    return bus->Read(address, false);
	    return 0x00;
	}


    void GbCpu::SetFlag(GbCpu::Flags flag){
        Registers.FlagsState |= flag;
    }
    void GbCpu::UnsetFlag(GbCpu::Flags flag){
        Registers.FlagsState &= ~flag;
    }

    bool GbCpu::IsFlagSet(GbCpu::Flags flag){
        return (Registers.FlagsState & flag);
    }

}