//
// Created by epat on 19.07.2021.
//

#ifndef GBEMU_LOADS_H
#define GBEMU_LOADS_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {

    void ExecLoadImmDataToRegister(uint16_t address, uint8_t value, std::string reg = "b"){
        int opcode = 0x06;
        reg = ToLower(reg);
        if(reg == "d"){
            opcode = 0x16;
        }
        else if(reg == "h"){
            opcode = 0x26;
        }

        SetOpcodeAndPC(address, opcode);

        Bus.Write(address + 1, value);

        RunNClockCycles(2);
    }

}

#endif //GBEMU_LOADS_H
