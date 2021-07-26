#include <iostream>
#include "GbCpu.h"



namespace Emulator {
    GbCpu::GbCpu()
    {
        SetOpcodes();
    }

    GbCpu::~GbCpu() = default;

    void GbCpu::Connect(Emulator::GbBus* bus) {
        this->Bus = bus;
    }

    void GbCpu::OnClockCycle() {
        if(InstructionsToResetIME-- == 0)
            InterruptMasterEnable = false;
        if(InstructionsToSetIME-- == 0)
            InterruptMasterEnable = true;
        if(InstructionsToResetIME < -1)
            InstructionsToResetIME = -1;
        if(InstructionsToSetIME < -1)
            InstructionsToSetIME = -1;

        if(InterruptMasterEnable && ExecutionSteps.empty() && ((GetInterruptFlags() & Bus->InterruptEnableRegister) > 0)) {
            PrepareForInterrupt();
        }
        else if(ExecutionSteps.empty()){
            auto f = Fetch();
            Opcodes[f].Exec();
        }


        if (!ExecutionSteps.empty()) {
            //Registers.PC++;
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
        ExecutionSteps.push([]{/*NOP*/});
        ExecutionSteps.push([]{/*NOP*/});
        ExecutionSteps.push([this]{ Fetched = Registers.PC; });
        ExecutionSteps.push([this] { StackPush(Fetched); });
        ExecutionSteps.push([this] {
            auto intrps = GetInterruptFlags() & Bus->InterruptEnableRegister;

            // clear corresponding flag and jump to vector
            if((intrps & (uint8_t)Interrupts::VerticalBlankInterrupt)) {
                SetInterruptFlags((Interrupts)(~(uint8_t)Interrupts::VerticalBlankInterrupt));
                Registers.PC = 0x0040;
            }
            else if((intrps & (uint8_t)Interrupts::LcdStatInterrupt)) {
                SetInterruptFlags((Interrupts)(~(uint8_t)Interrupts::LcdStatInterrupt));
                Registers.PC = 0x0048;
            }
            else if((intrps & (uint8_t)Interrupts::TimerInterrupt)) {
                SetInterruptFlags((Interrupts)(~(uint8_t)Interrupts::TimerInterrupt));
                Registers.PC = 0x0050;
            }
            else if((intrps & (uint8_t)Interrupts::SerialInterrupt)) {
                SetInterruptFlags((Interrupts)(~(uint8_t)Interrupts::SerialInterrupt));
                Registers.PC = 0x0058;
            }
            else if((intrps & (uint8_t)Interrupts::JoypadInterrupt)) {
                SetInterruptFlags((Interrupts)(~(uint8_t)Interrupts::JoypadInterrupt));
                Registers.PC = 0x0060;
            }
        });
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
        return (Registers.FlagsState & flag) > 0;
    }

    uint8_t GbCpu::GetInterruptFlags() {
        return Read(0xFF0F) & 0b00011111; // only lower 5 bits are interrupt flags
    }

    inline void GbCpu::SetInterruptFlags(Interrupts val) {
        SetInterruptFlags((uint8_t)val);
    }

    inline void GbCpu::SetInterruptFlags(uint8_t val){
        auto curVal = GetInterruptFlags();
        Write(0xFF0F, curVal & val);
    }

//    void GbCpu::StackPush(uint16_t value) {
//            auto high = (uint8_t)((value & 0xFF00) >> 8);
//            auto low = (uint8_t)(value & 0xFF);
//        Write(--Registers.SP, low);
//        Write(--Registers.SP, high);
//    }

    void GbCpu::StackPush(uint8_t value) {
        Write(--Registers.SP, value);
    }

//    uint16_t GbCpu::StackPop16() {
//        uint16_t low = Read(Registers.SP++);
//        uint16_t high = Read(Registers.SP++);
//        return (high << 8) | low;
//    }

    uint8_t GbCpu::StackPop() {
        return Read(Registers.SP++);
    }

    uint8_t GbCpu::Fetch() {
        return Read(Registers.PC++);
    }




}