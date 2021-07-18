//
// Created by epat on 17.07.2021.
//

#include "GbCpu.h"

namespace Emulator{
    namespace {
        typedef void     (CpuRegisters::*SetDoubleRegisterPtr) (uint16_t data);
        typedef uint16_t (CpuRegisters::*GetDoubleRegisterPtr) ();
        typedef void     (CpuRegisters::*SetRegisterPtr)       (uint8_t data);
        typedef uint8_t  (CpuRegisters::*GetRegisterPtr)       ();
    }
    void GbCpu::SetOpcodes() {


#pragma region  std::function templates ( or in other words, crimes against humanity )
        // I feel like that function and bindings below are crimes against humanity or something
        //buuut they work
        // for LD XX, d16
        std::function<void(SetDoubleRegisterPtr)> setDoubleRegisterToImmediateData
        ([this] (SetDoubleRegisterPtr setRegister) {
            ExecutionSteps.push([this] { Fetched = (uint16_t)Fetch(); });
            ExecutionSteps.push([this] { Fetched = Fetched | ((uint16_t)Fetch()) << 8; });
            ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched); });
        });

        std::function<void(GetDoubleRegisterPtr)> writeDataFromAddressToAccumulator
        ([this] (GetDoubleRegisterPtr getRegister) {
            ExecutionSteps.push([this, getRegister]{ Fetched = std::invoke(getRegister, Registers); });
            ExecutionSteps.push([this]{ Write(Fetched, Registers.Accumulator);});
        });

        std::function<void(GetDoubleRegisterPtr, SetDoubleRegisterPtr)> incrementDoubleRegister
        ([this] (GetDoubleRegisterPtr getRegister, SetDoubleRegisterPtr setRegister) {
            ExecutionSteps.push([this, getRegister] { Fetched = std::invoke(getRegister, Registers); });
            ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched + 1); });
        });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> incrementRegister
        ([this] (GetRegisterPtr getRegister, SetRegisterPtr setRegister) {
            ExecutionSteps.push([this, getRegister, setRegister]{
                uint8_t val = std::invoke(getRegister, Registers) + 1;
                std::invoke(setRegister, Registers, val);
                val == 0                ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                (val & 0b00001111) == 0 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Subtraction);
            });
        });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> decrementRegister
                ([this] (GetRegisterPtr getRegister, SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, setRegister]{
                        uint8_t val = std::invoke(getRegister, Registers) - 1;
                        std::invoke(setRegister, Registers, val);
                        val == 0                         ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                        (val & 0b00001111) != 0b00001111 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                        SetFlag(Flags::Subtraction);
                    });
                });

        std::function<void(SetRegisterPtr)> loadDataFromAddressToRegister
                ([this] (SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this] { Fetched = Fetch(); });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched); });
                } );


#pragma endregion

        Opcodes[0x00] = {"NOP",  [this]{ ExecutionSteps.push([]{}); }};

        Opcodes[0x10] = {"STOP", [this]{ ExecutionSteps.push([this]{
                Fetch();
                Bus->Clock.Stop();
                //TODO: stop LCD
            });
        }};

        Opcodes[0x20] = {"JR NZ,", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(!IsFlagSet(Flags::Zero)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }

        }};

        Opcodes[0x30] = {"JR NC,", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(!IsFlagSet(Flags::Carry)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }

        }};


        Opcodes[0x01] = {
                "LD BC,",
                std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetBC)
        };
        Opcodes[0x11] = {
                "LD DE,",
                std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetDE)
        };
        Opcodes[0x21] = {
                "LD HL,",
                std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetHL)
        };
        Opcodes[0x31] = {
                "LD SP,",
                std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetSP)
        };

                // TODO: do something about that additional text
//                [this] {
//                    return "";//Tools::StringConverters::GetHexString(Read(Registers.PC + 2)) +
//                    //Tools::StringConverters::GetHexString(Read(Registers.PC + 1));
//                }};




        Opcodes[0x02] = {"LD (BC), A", std::bind(writeDataFromAddressToAccumulator, &CpuRegisters::GetBC) };
        Opcodes[0x12] = {"LD (DE), A", std::bind(writeDataFromAddressToAccumulator, &CpuRegisters::GetDE) };
        Opcodes[0x22] = {"LD (HL+), A", [this]  {
            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched + 1); });
            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
        }};
        Opcodes[0x32] = {"LD (HL-), A", [this]  {
            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched - 1); });
            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
        }};




        Opcodes[0x03] = { "INC BC", std::bind(incrementDoubleRegister, &CpuRegisters::GetBC, &CpuRegisters::SetBC) };
        Opcodes[0x13] = { "INC DE", std::bind(incrementDoubleRegister, &CpuRegisters::GetDE, &CpuRegisters::SetDE) };
        Opcodes[0x23] = { "INC HL", std::bind(incrementDoubleRegister, &CpuRegisters::GetHL, &CpuRegisters::SetHL) };
        Opcodes[0x33] = { "INC SP", std::bind(incrementDoubleRegister, &CpuRegisters::GetSP, &CpuRegisters::SetSP) };




        Opcodes[0x04] = {"INC B", std::bind(incrementRegister, &CpuRegisters::GetB, &CpuRegisters::SetB )};
        Opcodes[0x14] = {"INC D", std::bind(incrementRegister, &CpuRegisters::GetD, &CpuRegisters::SetD )};
        Opcodes[0x24] = {"INC H", std::bind(incrementRegister, &CpuRegisters::GetH, &CpuRegisters::SetH )};
        Opcodes[0x34] = {"INC (HL)", [this]{
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Fetched2 = Read(Fetched) + 1; });
            ExecutionSteps.push([this] {
                Write(Fetched, Fetched2);
                uint8_t val = (uint8_t)Fetched2;
                val == 0                ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                (val & 0b00001111) == 0 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Subtraction);
            });
        }};

        Opcodes[0x05] = {"DEC B", std::bind(decrementRegister, &CpuRegisters::GetB, &CpuRegisters::SetB )};
        Opcodes[0x15] = {"DEC D", std::bind(decrementRegister, &CpuRegisters::GetD, &CpuRegisters::SetD )};
        Opcodes[0x25] = {"DEC H", std::bind(decrementRegister, &CpuRegisters::GetH, &CpuRegisters::SetH )};
        Opcodes[0x34] = {"DEC (HL)", [this]{
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Fetched2 = Read(Fetched) - 1; });
            ExecutionSteps.push([this] {
                Write(Fetched, Fetched2);
                uint8_t val = (uint8_t)Fetched2;
                val == 0                     ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                (val & 0b00001111) != 0b1111 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                SetFlag(Flags::Subtraction);
            });
        }};



        Opcodes[0x06] = { "LD B,", std::bind(loadDataFromAddressToRegister, &CpuRegisters::SetB)};
        Opcodes[0x16] = { "LD D,", std::bind(loadDataFromAddressToRegister, &CpuRegisters::SetD)};
        Opcodes[0x26] = { "LD H,", std::bind(loadDataFromAddressToRegister, &CpuRegisters::SetH)};
        Opcodes[0x36] = {"LD (HL),", [this]{
            ExecutionSteps.push([this] { Fetched2 = Fetch(); });
            ExecutionSteps.push([this] { Fetched  = Registers.GetHL(); });
            ExecutionSteps.push([this] { Write(Fetched, Fetched2); });
        }};

        Opcodes[0x07] = {"RLCA", [this] { ExecutionSteps.push([this]{
            uint8_t val   = Registers.GetA();
            uint8_t carry = (val & (1 << 7)) >> 7;
            val = (val << 1) | carry;
            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            UnsetFlag(Flags::Zero);
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
        }); }};
        Opcodes[0x17] = {"RLA", [this] { ExecutionSteps.push([this]{
            uint8_t val   = Registers.GetA();
            uint8_t carry = (val & (1 << 7)) >> 7;
            bool carrySet = IsFlagSet(Flags::Carry);
            val = (val << 1) | (carrySet ? 1 : 0);

            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            UnsetFlag(Flags::Zero);
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
        }); }};
    }
}