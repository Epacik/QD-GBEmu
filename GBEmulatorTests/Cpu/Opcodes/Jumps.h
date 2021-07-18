//
// Created by epat on 18.07.2021.
//

#ifndef GBEMU_JUMPS_H
#define GBEMU_JUMPS_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {
    void ExecJrNz(uint16_t address, uint8_t jumpBy, bool setZeroFlag ) {
        SetOpcodeAndPC(address, 0x20);
        Bus.Write(address + 1, jumpBy);

        using namespace Emulator;
        setZeroFlag ? Bus.Cpu->SetFlag(GbCpu::Flags::Zero) : Bus.Cpu->UnsetFlag(GbCpu::Flags::Zero);

        RunNClockCycles(setZeroFlag ? 2 : 3);
    };

    void ExecJrNc(uint16_t address, uint8_t jumpBy, bool setCarryFlag ) {
        SetOpcodeAndPC(address, 0x30);
        Bus.Write(address + 1, jumpBy);

        using namespace Emulator;
        setCarryFlag ? Bus.Cpu->SetFlag(GbCpu::Flags::Carry) : Bus.Cpu->UnsetFlag(GbCpu::Flags::Carry);

        RunNClockCycles(setCarryFlag ? 2 : 3);
    };

}


#endif //GBEMU_JUMPS_H
