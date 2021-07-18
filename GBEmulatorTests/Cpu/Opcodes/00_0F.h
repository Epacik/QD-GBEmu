//
// Created by epat on 15.07.2021.
//

#ifndef GBEMU_00_0F_H
#define GBEMU_00_0F_H

#include "../../../GBEmulator/Emu/GbBus.h"

namespace Cpu::Opcodes {


    void ExecuteNop(uint16_t address){
        SetOpcodeAndPC(address, 0x00);

        Bus.Cpu->OnClockCycle();
    }

    void ExecLdDoubleD16(uint16_t address, uint8_t b1, uint8_t b2, std::string reg = "bc"){
        int opcode = 0x01;
        reg = ToLower(reg);
        if(reg == "de"){
            opcode = 0x11;
        }
        else if(reg == "hl"){
            opcode = 0x21;
        }
        else if(reg == "sp"){
            opcode = 0x31;
        }
        SetOpcodeAndPC(address, opcode);

        Bus.Write(Bus.Cpu->Registers.PC + 1, b2);
        Bus.Write(Bus.Cpu->Registers.PC + 2, b1);

        RunNClockCycles(3);
//        Bus.Cpu->OnClockCycle();
//        Bus.Cpu->OnClockCycle();
//        Bus.Cpu->OnClockCycle();
    }

    void ExecLdRegAddrA(uint16_t address, uint8_t A, uint16_t BC, std::string reg = "bc") {

        int opcode = 0x02;
        reg = ToLower(reg);
        if(reg == "de"){
            opcode = 0x12;
            Bus.Cpu->Registers.SetDE(BC);
        }
        else if(reg == "hl+"){
            opcode = 0x22;
            Bus.Cpu->Registers.SetHL(BC);
        }
        else if(reg == "hl-"){
            opcode = 0x32;
            Bus.Cpu->Registers.SetHL(BC);
        }
        else{
            Bus.Cpu->Registers.SetBC(BC);
        }

        SetOpcodeAndPC(address, opcode);

        Bus.Cpu->Registers.SetA(A);


        RunNClockCycles(2);
//        Bus.Cpu->OnClockCycle();
//        Bus.Cpu->OnClockCycle();
    }

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

    void ExecIncRegister(uint16_t address, uint8_t initialValue){
        SetOpcodeAndPC(address, 0x04);
        Bus.Cpu->Registers.SetB(initialValue);
        Bus.Cpu->OnClockCycle();
    }

    void ExecDecB(uint16_t address, uint8_t initialValue){
        SetOpcodeAndPC(address, 0x05);
        Bus.Cpu->Registers.SetB(initialValue);
        Bus.Cpu->OnClockCycle();
    }

}


#endif //GBEMU_00_0F_H
