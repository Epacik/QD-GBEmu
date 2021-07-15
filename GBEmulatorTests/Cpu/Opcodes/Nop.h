//
// Created by epat on 15.07.2021.
//

#ifndef GBEMU_NOP_H
#define GBEMU_NOP_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {
    Emulator::GbBus Bus(true);

    uint16_t TestNop(uint16_t address){
        Bus.Cpu->Registers.PC = address;
        Bus.Write(Bus.Cpu->Registers.PC, 0x00);
        Bus.Cpu->OnClockCycle();
        return Bus.Cpu->Registers.PC;
    }
}


#endif //GBEMU_NOP_H
