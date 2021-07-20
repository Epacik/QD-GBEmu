//
// Created by epat on 19.07.2021.
//

#ifndef GBEMU_DECREMENTS_H
#define GBEMU_DECREMENTS_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes{
    void ExecDecRegister(uint16_t address, uint8_t initialValue, std::string reg = "b"){
        int opcode = 0x05;
        reg = ToLower(reg);
        if(reg == "d"){
            opcode = 0x15;
            Bus.Cpu->Registers.SetD(initialValue);
        }
        else if(reg == "h"){
            opcode = 0x25;
            Bus.Cpu->Registers.SetH(initialValue);
        }
        else{
            Bus.Cpu->Registers.SetB(initialValue);
        }

        SetOpcodeAndPC(address, opcode);
        Bus.Cpu->OnClockCycle();
    }

    void ExecDecMemAtAt(uint16_t address, uint16_t registerValue, uint8_t initialValue){
        SetOpcodeAndPC(address, 0x35);
        Bus.Cpu->Registers.SetHL(registerValue);
        Bus.Write(registerValue, initialValue);
        RunNClockCycles(3);
    }
}

#endif //GBEMU_DECREMENTS_H
