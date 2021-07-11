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
        uint8_t Accumulator = 0x01;
        uint8_t FlagsState = 0xB0;

        uint16_t get_BC() {
            return (((uint16_t) B) << 8) | C;
        }

        uint8_t B = 0x00;
        uint8_t C = 0x13;

        uint16_t get_DE() {
            return (((uint16_t) D) << 8) | E;
        }

        uint8_t D = 0x00;
        uint8_t E = 0xD8;

        uint16_t get_HL() {
            return (((uint16_t) H) << 8) | L;
        }

        uint8_t H = 0x01;
        uint8_t L = 0x4D;

        uint16_t SP = 0xFFFE;
        uint16_t PC = 0x0100;

    };

}
#endif //GBEMU_CPUREGISTERS_H
