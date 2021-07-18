//
// Created by epat on 18.07.2021.
//

#ifndef GBEMU_TOOLS_H
#define GBEMU_TOOLS_H

#include "../../../GBEmulator/Emu/GbBus.h"


namespace Cpu::Opcodes{
    Emulator::GbBus Bus(true);

    void SetOpcodeAndPC(uint16_t address, uint8_t opcode){
        Bus.Write(address, opcode);
        Bus.Cpu->Registers.PC = address;
    }

    void RunNClockCycles(int n){
        for (int i = 0; i < n; i++) {
            Bus.Cpu->OnClockCycle();
        }
    }

    std::string ToLower(std::string data){
        std::transform(data.begin(), data.end(), data.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return data;
    }
}

#endif //GBEMU_TOOLS_H
