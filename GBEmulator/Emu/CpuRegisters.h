//
// Created by epat on 09.07.2021.
//

#ifndef GBEMU_CPUREGISTERS_H
#define GBEMU_CPUREGISTERS_H

#include <cstdint>
#include <wx/string.h>
#include <bitset>
#include <string>

namespace Emulator {
    class CpuRegisters {
    public:
        uint16_t GetAF();
        uint8_t GetA();
        uint8_t GetF();
        void SetAF(uint16_t data);
        void SetA(uint8_t data);
        void SetF(uint8_t data);

        uint8_t Accumulator = 0x01;
        uint8_t FlagsState = 0xB0;


        uint16_t GetBC();
        uint8_t GetB();
        uint8_t GetC();
        void SetBC(uint16_t data);
        void SetB(uint8_t data);
        void SetC(uint8_t data);

        uint8_t B = 0x00;
        uint8_t C = 0x13;


        uint16_t GetDE();
        uint8_t GetD();
        uint8_t GetE();
        void SetDE(uint16_t data);
        void SetD(uint8_t data);
        void SetE(uint8_t data);

        uint8_t D = 0x00;
        uint8_t E = 0xD8;



        uint16_t GetHL();
        uint8_t GetH();
        uint8_t GetL();
        void SetHL(uint16_t data);
        void SetH(uint8_t data);
        void SetL(uint8_t data);

        uint8_t H = 0x01;
        uint8_t L = 0x4D;

        uint16_t SP = 0xFFFE;
        uint16_t PC = 0x0100;

    };

}
#endif //GBEMU_CPUREGISTERS_H
