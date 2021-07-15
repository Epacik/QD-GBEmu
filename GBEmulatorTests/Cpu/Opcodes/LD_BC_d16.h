//
// Created by epat on 15.07.2021.
//

#ifndef GBEMU_LD_BC_D16_H
#define GBEMU_LD_BC_D16_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {

    uint16_t TestLdBcD16(uint16_t address, uint8_t b1, uint8_t b2){
        Bus.Cpu->Registers.PC = address;
        Bus.Write(Bus.Cpu->Registers.PC, 0x01);
        Bus.Write(Bus.Cpu->Registers.PC + 1, b2);
        Bus.Write(Bus.Cpu->Registers.PC + 2, b1);
        Bus.Cpu->OnClockCycle();
        Bus.Cpu->OnClockCycle();
        Bus.Cpu->OnClockCycle();
        return Bus.Cpu->Registers.GetBC();
    }
}

#endif //GBEMU_LD_BC_D16_H
