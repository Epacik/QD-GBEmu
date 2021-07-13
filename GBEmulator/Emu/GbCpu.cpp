#include <iostream>
#include "GbCpu.h"


namespace Emulator {
	GbCpu::GbCpu()
	{

	}

	GbCpu::~GbCpu() = default;

	void GbCpu::Connect(Emulator::GbBus* bus) {
        this->Bus = bus;
	}

    bool executingInstruction = false; // temporary
    void GbCpu::OnClockCycle() {
        if(InterruptMasterEnable && !executingInstruction && ((GetInterruptFlags() & Bus->InterruptEnableRegister) > 0)) {
            PrepareForInterrupt();
        }

        if(!ExecutionSteps.empty()){
            ExecutionSteps.front()();
            ExecutionSteps.pop();
        }

    }

    void GbCpu::PrepareForInterrupt() {
        InterruptMasterEnable = false;

        // Interrupt Service Routine
        //According to Z80 datasheets, the following occurs when control is being transferred to an interrupt
        //handler:
        //1. Two wait states are executed (2 machine cycles pass while nothing occurs, presumably the CPU
        //is executing NOPs during this time).
        //2. The current PC is pushed onto the stack, this process consumes 2 more machine cycles.
        //3. The high byte of the PC is set to 0, the low byte is set to the address of the handler
        //($40,$48,$50,$58,$60). This consumes one last machine cycle.

        uint16_t pc = 0x00;

        //one step per clock cycle
        ExecutionSteps.push([]{});
        ExecutionSteps.push([]{});
        ExecutionSteps.push([&pc, this]{ pc = Registers.PC; });
        ExecutionSteps.push([pc, this] { StackPush(pc); });
        ExecutionSteps.push([this] {
            auto intrps = GetInterruptFlags() & Bus->InterruptEnableRegister;

            if((intrps & (uint8_t)Interrupts::VerticalBlankInterrupt)) {
                Registers.PC = 0x0040;
            }
            else if((intrps & (uint8_t)Interrupts::LcdStatInterrupt)) {
                Registers.PC = 0x0048;
            }
            else if((intrps & (uint8_t)Interrupts::TimerInterrupt)) {
                Registers.PC = 0x0050;
            }
            else if((intrps & (uint8_t)Interrupts::SerialInterrupt)) {
                Registers.PC = 0x0058;
            }
            else if((intrps & (uint8_t)Interrupts::JoypadInterrupt)) {
                Registers.PC = 0x0060;
            }
            
            executingInstruction = false;
        });

        executingInstruction = true;
    }


	void GbCpu::Write(uint16_t address, uint8_t data)
	{
	    if(Bus != nullptr)
		    Bus->Write(address, data);
	}

	uint8_t GbCpu::Read(uint16_t address)
	{
	    if(Bus != nullptr)
		    return Bus->Read(address, false);
	    return 0x00;
	}


    void GbCpu::SetFlag(GbCpu::Flags flag){
        Registers.FlagsState |= flag;
    }
    void GbCpu::UnsetFlag(GbCpu::Flags flag){
        Registers.FlagsState &= ~flag;
    }

    bool GbCpu::IsFlagSet(GbCpu::Flags flag) {
        return (Registers.FlagsState & flag);
    }

    uint8_t GbCpu::GetInterruptFlags() {
        return Read(0xFF0F) & 0b00011111; // only lower 5 bits are interrupt flags
    }

    uint8_t GbCpu::StackPop() {
        return Read(Registers.SP++);
    }

    void GbCpu::StackPush(uint8_t value) {
        Write(Registers.SP--, value);
    }


}