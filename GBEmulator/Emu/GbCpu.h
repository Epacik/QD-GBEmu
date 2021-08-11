#pragma once
#ifndef CPU_H
#define CPU_H



#include <queue>
#include <functional>

#include "GbBus.h"
#include "CpuRegisters.h"
#include "Instruction.h"
#include "Interrupts.h"


namespace Emulator
{


    class GbBus;
    class GbCpu
    {
    public:
        GbCpu();
        ~GbCpu();

    private:
        void Write(uint16_t address, uint8_t data);
        uint8_t Read(uint16_t address);

    public:
        uint8_t GetInterruptFlags();
        void SetInterruptFlag(Interrupts val);
        void UnsetInterruptFlag(Interrupts val);

    private:
        void SetInterruptFlag(uint8_t val);
        void UnsetInterruptFlag(uint8_t val);
            
        bool InterruptMasterEnable = true; // false if executing an interrupt handler
        bool Halt = false;
        bool Stop = false;
        uint16_t Fetched  = 0x0000;
        uint16_t Fetched2 = 0x0000;

        void StackPush(uint8_t value);
        uint8_t StackPop();

        uint8_t Fetch();

        void PrepareForInterrupt();

        // contains steps required for an instruction to work
        std::queue<std::function<void()>> ExecutionSteps;

        int InstructionsToResetIME = -1;
        int InstructionsToSetIME = -1;

        bool IMEBack = false;


    public:
        enum Flags {
            Zero        = (1 << 7),
            Subtraction = (1 << 6), // BCD
            HalfCarry   = (1 << 5), // BCD
            Carry       = (1 << 4),
        };

        CpuRegisters Registers;

        void SetFlag(Flags flag);
        void UnsetFlag(Flags flag);
        bool IsFlagSet(Flags flag);
        void OnClockCycle();
        
    public:
        void Connect(GbBus* bus);
    protected:
        GbBus* Bus;

    private:
        void SetOpcodes();
        void SetExtendedOpcodes();

        //Opcodes
        std::unordered_map<uint8_t, Opcode> Opcodes;
        std::unordered_map<uint8_t, Opcode> OpcodesEx;
    };
}


#endif // !CPU_H