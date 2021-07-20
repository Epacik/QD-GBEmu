//
// Created by epat on 19.07.2021.
//

#ifndef GBEMU_ROTATIONS_H
#define GBEMU_ROTATIONS_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {
    void ExecRLCA(uint16_t address, uint8_t value){
        SetOpcodeAndPC(address, 0x07);
        Bus.Cpu->Registers.SetA(value);
        Bus.Cpu->OnClockCycle();
    }
    void ExecRLA(uint16_t address, uint8_t value){
        SetOpcodeAndPC(address, 0x17);
        Bus.Cpu->Registers.SetA(value);
        Bus.Cpu->OnClockCycle();
    }

}

#endif //GBEMU_ROTATIONS_H
