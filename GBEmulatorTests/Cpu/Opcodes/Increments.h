//
// Created by epat on 19.07.2021.
//

#ifndef GBEMU_INCREMENTS_H
#define GBEMU_INCREMENTS_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes{

    void ExecIncDoubleRegister(uint16_t address, uint16_t initialValue = 0x0013, std::string reg = "bc") {
        int opcode = 0x03;
        reg = ToLower(reg);
        if(reg == "de"){
            opcode = 0x13;
            Bus.Cpu->Registers.SetDE(initialValue);
        }
        else if(reg == "hl"){
            opcode = 0x23;
            Bus.Cpu->Registers.SetHL(initialValue);
        }
        else if(reg == "sp"){
            opcode = 0x33;
            Bus.Cpu->Registers.SetSP(initialValue);
        }
        else{
            Bus.Cpu->Registers.SetBC(initialValue);
        }

        SetOpcodeAndPC(address, opcode);



        RunNClockCycles(2);
    }

    void ExecIncRegister(uint16_t address, uint8_t initialValue, std::string reg = "b"){
        int opcode = 0x04;
        reg = ToLower(reg);
        if(reg == "d"){
            opcode = 0x14;
            Bus.Cpu->Registers.SetD(initialValue);
        }
        else if(reg == "h"){
            opcode = 0x24;
            Bus.Cpu->Registers.SetH(initialValue);
        }
        else{
            Bus.Cpu->Registers.SetB(initialValue);
        }

        SetOpcodeAndPC(address, opcode);
        Bus.Cpu->OnClockCycle();
    }

    void ExecIncMemAtAt(uint16_t address, uint16_t registerValue, uint8_t initialValue){
        SetOpcodeAndPC(address, 0x34);
        Bus.Cpu->Registers.SetHL(registerValue);
        Bus.Write(registerValue, initialValue);
        RunNClockCycles(3);
    }
}

#endif //GBEMU_INCREMENTS_H
