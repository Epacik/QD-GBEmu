#pragma once
#ifndef CPU_H
#define CPU_H



#include <queue>
#include <functional>

#include "GbBus.h"
#include "CpuRegisters.h"
#include "Instruction.h"



namespace Emulator
{
    class GbBus;

    class GbCpu
    {
    public:
        GbCpu();
        ~GbCpu();

        void Connect(GbBus* bus);

    enum class Interrupts {
        VerticalBlankInterrupt = 1,
        LcdStatInterrupt       = 1 << 1,
        TimerInterrupt         = 1 << 2,
        SerialInterrupt        = 1 << 3,
        JoypadInterrupt        = 1 << 4,
    };

    private:
        GbBus* Bus = nullptr;

        void Write(uint16_t address, uint8_t data);
        uint8_t Read(uint16_t address);

        uint8_t GetInterruptFlags();
        void SetInterruptFlags(Interrupts val);
        void SetInterruptFlags(uint8_t val);

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

        private:
        void SetOpcodes();
        void SetExtendedOpcodes();

        //Opcodes
        std::unordered_map<uint8_t, Opcode> Opcodes;
        std::unordered_map<uint8_t, Opcode> OpcodesEX;
    };
}


#endif // !CPU_H