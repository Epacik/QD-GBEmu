//
// Created by epat on 09.07.2021.
//

#include "CpuRegisters.h"
namespace Emulator {
    uint16_t CpuRegisters::GetAF() {
        return ((uint16_t)Accumulator << 8 | (uint16_t)FlagsState);
    }

    uint8_t CpuRegisters::GetA() {
        return Accumulator;
    }

    uint8_t CpuRegisters::GetF() {
        return FlagsState;
    }

    void CpuRegisters::SetAF(uint16_t data) {
        FlagsState  = (uint8_t)(data & 0xf0);
        Accumulator = (uint8_t)((data & 0xff00) >> 8);
    }

    void CpuRegisters::SetA(uint8_t data) {
        Accumulator = data;
    }

    void CpuRegisters::SetF(uint8_t data) {
        FlagsState = data & 0xF0;
    }



    uint16_t CpuRegisters::GetBC() {
        return ((uint16_t)B << 8 | (uint16_t)C);
    }
    uint8_t CpuRegisters::GetB() {
        return B;
    }

    uint8_t CpuRegisters::GetC() {
        return C;
    }

    void CpuRegisters::SetBC(uint16_t data) {
        C  = (uint8_t)(data & 0xff);
        B = (uint8_t)((data & 0xff00) >> 8);
    }

    void CpuRegisters::SetB(uint8_t data) {
        B = data;
    }

    void CpuRegisters::SetC(uint8_t data) {
        C = data & 0xF0;
    }


    uint16_t CpuRegisters::GetDE() {
        return ((uint16_t)D << 8 | (uint16_t)E);
    }
    uint8_t CpuRegisters::GetD() {
        return D;
    }

    uint8_t CpuRegisters::GetE() {
        return E;
    }

    void CpuRegisters::SetDE(uint16_t data) {
        E  = (uint8_t)(data & 0xff);
        D = (uint8_t)((data & 0xff00) >> 8);
    }

    void CpuRegisters::SetD(uint8_t data) {
        D = data;
    }

    void CpuRegisters::SetE(uint8_t data) {
        E = data & 0xF0;
    }



    uint16_t CpuRegisters::GetHL() {
        return ((uint16_t)H << 8 | (uint16_t)L);
    }
    uint8_t CpuRegisters::GetH() {
        return H;
    }

    uint8_t CpuRegisters::GetL() {
        return L;
    }

    void CpuRegisters::SetHL(uint16_t data) {
        L  = (uint8_t)(data & 0xff);
        H = (uint8_t)((data & 0xff00) >> 8);
    }

    void CpuRegisters::SetH(uint8_t data) {
        H = data;
    }

    void CpuRegisters::SetL(uint8_t data) {
        L = data & 0xF0;
    }

    void CpuRegisters::SetSP(uint16_t data) {
        SP = data;
    }

    void CpuRegisters::SetPC(uint16_t data) {
        PC = data;
    }

    uint16_t CpuRegisters::GetSP() {
        return SP;
    }

    uint16_t CpuRegisters::GetPC() {
        return PC;
    }
}