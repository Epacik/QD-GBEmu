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
        std::function<void(uint8_t, uint8_t)>   HalfCarryHelper  ([this](uint8_t r1, uint8_t r2){
            if((((r1 & 0xf) + (r2 & 0xf)) & 0x10) == 0x10 ){
                SetFlag(Flags::HalfCarry);
            }
            else{
                UnsetFlag(Flags::HalfCarry);
            }
        });
        std::function<void(uint16_t, uint16_t)> HalfCarryHelper16([this, HalfCarryHelper](uint16_t r1, uint16_t r2){
            uint8_t r1u = (uint8_t)((r1 & 0xFF00) >> 8);
            uint8_t r2u = (uint8_t)((r2 & 0xFF00) >> 8);
            HalfCarryHelper(r1u, r2u);
        });

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

        std::function<void(GetDoubleRegisterPtr, GetRegisterPtr, int, SetDoubleRegisterPtr)> writeDataFromRegisterToAddress
        ([this] (GetDoubleRegisterPtr getRegisterAddr, GetRegisterPtr getRegister, int incrementBy, SetDoubleRegisterPtr setRegister) {
            ExecutionSteps.push([this, getRegisterAddr]{ Fetched = std::invoke(getRegisterAddr, Registers); });
            ExecutionSteps.push([this, getRegister, incrementBy, setRegister]{
                Write(Fetched, std::invoke(getRegister, Registers));
                if(incrementBy != 0) {
                    std::invoke(setRegister, Registers, Fetched + incrementBy);
                }
            });
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

        std::function<void(SetRegisterPtr)> loadDataFromAddressToRegisterImmediate
        ([this] (SetRegisterPtr setRegister) {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched); });
        } );

        std::function<void(GetDoubleRegisterPtr, GetDoubleRegisterPtr, SetDoubleRegisterPtr)> AddDoubleRegisters
        ([this, HalfCarryHelper16] (GetDoubleRegisterPtr getRegister, GetDoubleRegisterPtr getRegister2, SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, getRegister2, setRegister, HalfCarryHelper16] {
                        uint32_t r1 = (uint32_t)std::invoke(getRegister,  Registers);
                        uint32_t r2 = (uint32_t)std::invoke(getRegister2, Registers);
                        uint32_t val = r1 + r2;

                        val > 0xFFFF ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        HalfCarryHelper16(r1, r2);

                        Fetched = (uint16_t)(val & 0xFFFF);
                        std::invoke(setRegister, Registers, (Fetched & 0xFF) | (r1 & 0xFF00));
                     });
                    ExecutionSteps.push([this, setRegister] {
                        std::invoke(setRegister, Registers, Fetched);
                        UnsetFlag(Flags::Subtraction);
                    });
                });

        std::function<void(GetDoubleRegisterPtr, SetRegisterPtr, int, SetDoubleRegisterPtr)> loadDataToRegisterByAddressInRegisterPair
        ([this] (GetDoubleRegisterPtr getRegister, SetRegisterPtr setRegister, int incrementBy, SetDoubleRegisterPtr setAddressReg){
            ExecutionSteps.push([this, getRegister, incrementBy, setAddressReg]{
                Fetched = (uint16_t)std::invoke(getRegister, Registers);
                if(incrementBy != 0)
                    std::invoke(setAddressReg, Registers, Fetched + incrementBy);
            });
            ExecutionSteps.push([this, setRegister]{ std::invoke(setRegister, Registers, Read(Fetched)); });
        });

        std::function<void(GetDoubleRegisterPtr, SetDoubleRegisterPtr)> decrementDoubleRegister
                ([this] (GetDoubleRegisterPtr getRegister, SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister] { Fetched = std::invoke(getRegister, Registers); });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched - 1); });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> loadDataFromOneRegisterToAnother
        ([this] (GetRegisterPtr getRegister, SetRegisterPtr setRegister){
           ExecutionSteps.push([this, getRegister, setRegister] { std::invoke(setRegister, Registers, std::invoke(getRegister, Registers)); });
        });

        std::function<void(GetRegisterPtr)> addRegisterToAccumulator
        ([this, HalfCarryHelper](GetRegisterPtr getRegister){
           ExecutionSteps.push([this, getRegister, HalfCarryHelper]{
               uint8_t reg = std::invoke(getRegister, Registers);
               UnsetFlag(Flags::Subtraction);
               HalfCarryHelper(reg, Registers.Accumulator);
               uint16_t result = (uint16_t)reg + (uint16_t)Registers.Accumulator;
               Registers.SetA((uint8_t)result & 0xFF);

               (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
               (result & 0xFF)  == 0 ? SetFlag(Flags::Zero)  : UnsetFlag(Flags::Zero);
           });
        });

        std::function<void(GetRegisterPtr)> addRegisterAndCarryToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister){
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper]{
                        uint8_t reg = std::invoke(getRegister, Registers);
                        UnsetFlag(Flags::Subtraction);
                        uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                        HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                        uint16_t result = (uint16_t)reg + (uint16_t)Registers.Accumulator + (uint16_t)carryVal;
                        Registers.SetA((uint8_t)result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF)  == 0 ? SetFlag(Flags::Zero)  : UnsetFlag(Flags::Zero);
                    });
                });

#pragma endregion

#pragma region 0x00 - 0x3F

        Opcodes[0x00] = {"NOP",  [this]{ ExecutionSteps.push([]{}); }};
        Opcodes[0x10] = {"STOP", [this]{ ExecutionSteps.push([this]{
                Fetch();
                Bus->Clock.Stop();
                //TODO: stop LCD
            });}};
        Opcodes[0x20] = {"JR NZ,", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(!IsFlagSet(Flags::Zero)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }}};
        Opcodes[0x30] = {"JR NC,", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(!IsFlagSet(Flags::Carry)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }}};

        Opcodes[0x01] = { "LD BC,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetBC) };
        Opcodes[0x11] = { "LD DE,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetDE) };
        Opcodes[0x21] = { "LD HL,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetHL) };
        Opcodes[0x31] = { "LD SP,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetSP) };

                // TODO: do something about that additional text
//                [this] {
//                    return "";//Tools::StringConverters::GetHexString(Read(Registers.PC + 2)) +
//                    //Tools::StringConverters::GetHexString(Read(Registers.PC + 1));
//                }};

        Opcodes[0x02] = {"LD (BC), A",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetBC, &CpuRegisters::GetA,  0, nullptr) };
        Opcodes[0x12] = {"LD (DE), A",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetDE, &CpuRegisters::GetA,  0, nullptr) };
        Opcodes[0x22] = {"LD (HL+), A", std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA,  1, &CpuRegisters::SetHL)};
        Opcodes[0x32] = {"LD (HL-), A", std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA, -1, &CpuRegisters::SetHL)};
//        Opcodes[0x22] = {"LD (HL+), A", [this]  {
//            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched + 1); });
//            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
//        }};
//        Opcodes[0x32] = {"LD (HL-), A", [this]  {
//            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched - 1); });
//            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
//        }};

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
                auto val = (uint8_t)Fetched2;
                val == 0                ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                (val & 0b00001111) == 0 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Subtraction);
            });
        }};

        Opcodes[0x05] = {"DEC B", std::bind(decrementRegister, &CpuRegisters::GetB, &CpuRegisters::SetB )};
        Opcodes[0x15] = {"DEC D", std::bind(decrementRegister, &CpuRegisters::GetD, &CpuRegisters::SetD )};
        Opcodes[0x25] = {"DEC H", std::bind(decrementRegister, &CpuRegisters::GetH, &CpuRegisters::SetH )};
        Opcodes[0x35] = {"DEC (HL)", [this]{
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Fetched2 = Read(Fetched) - 1; });
            ExecutionSteps.push([this] {
                Write(Fetched, Fetched2);
                auto val = (uint8_t)Fetched2;
                val == 0                     ? SetFlag(Flags::Zero)      : UnsetFlag(Flags::Zero);
                (val & 0b00001111) != 0b1111 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                SetFlag(Flags::Subtraction);
            });
        }};

        Opcodes[0x06] = { "LD B,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetB)};
        Opcodes[0x16] = { "LD D,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetD)};
        Opcodes[0x26] = { "LD H,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetH)};
        Opcodes[0x36] = { "LD (HL),", [this]{
            ExecutionSteps.push([this] { Fetched2 = Fetch(); });
            ExecutionSteps.push([this] { Fetched  = Registers.GetHL(); });
            ExecutionSteps.push([this] { Write(Fetched, Fetched2); });
        }};

        Opcodes[0x07] = { "RLCA", [this] { ExecutionSteps.push([this]{
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
            uint8_t val   = Registers.GetA();
            if(val == 0) { SetFlag(Flags::Zero); return; }
            uint8_t carry = (val & (1 << 7)) >> 7;
            val = (val << 1) | carry;
            Registers.SetA(val);
            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            UnsetFlag(Flags::Zero);
        }); }};
        Opcodes[0x17] = { "RLA",  [this] { ExecutionSteps.push([this]{
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
            uint8_t val   = Registers.GetA();
            uint8_t carry = (val & (1 << 7)) >> 7;
            bool carrySet = IsFlagSet(Flags::Carry);
            val = (val << 1) | (carrySet ? 1 : 0);

            Registers.SetA(val);

            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

        }); }};
        Opcodes[0x27] = { "DAA",  [this] { ExecutionSteps.push([this]{
            //TODO: Find something to test that thing correctly
            UnsetFlag(Flags::HalfCarry);
            uint8_t val = Registers.GetA();
            uint8_t cor = 0x00;
            bool setCarry = false;
            if(IsFlagSet(Flags::HalfCarry) || (!IsFlagSet(Flags::Subtraction) && (val & 0xf) > 9))
                cor = 0x06;

            if(IsFlagSet(Flags::Carry) || (!IsFlagSet(Flags::Subtraction) && (val & 0xf) > 9))
            {
                cor |= 0x60;
                setCarry = IsFlagSet(Flags::Carry);
            }

            IsFlagSet(Flags::Subtraction) ? val -= cor : val += cor;

            val == 0 ? SetFlag(Flags::Zero)  : UnsetFlag(Flags::Zero);
            setCarry ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);

            Registers.SetA(val);
        }); }};
        Opcodes[0x37] = { "SCF",  [this] { ExecutionSteps.push([this] { SetFlag(Flags::Carry); }); }};

        Opcodes[0x08] = { "LD (a16), SP", [this]{
            ExecutionSteps.push([this]{ Fetched  =  (uint16_t)Fetch(); });
            ExecutionSteps.push([this]{ Fetched  |= ((uint16_t)Fetch() << 8); });
            ExecutionSteps.push([this]{ Fetched2 =  Registers.GetSP(); });
            ExecutionSteps.push([this]{ Fetched2 =  (uint8_t)(Fetched2 & 0xFF); });
            ExecutionSteps.push([this]{ Write(Fetched, (uint8_t)Fetched2); });
        } };
        Opcodes[0x18] = {"JR s8", [this]{
            ExecutionSteps.push([this]{ Fetched = Fetch(); });
            ExecutionSteps.push([]{});
            ExecutionSteps.push([this]{ Registers.PC += (int8_t)Fetched; });
        }};
        Opcodes[0x28] = {"JR Z, s8", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(IsFlagSet(Flags::Zero)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }}};
        Opcodes[0x38] = {"JR C, s8", [this]{
            ExecutionSteps.push([this]{ Fetched = (uint8_t)Fetch(); });
            ExecutionSteps.push([]{});
            if(IsFlagSet(Flags::Carry)){
                ExecutionSteps.push([this] { Registers.PC += (int8_t)Fetched; });
            }}};

        Opcodes[0x09] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetBC, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x19] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetDE, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x29] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetHL, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x39] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetSP, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};

        Opcodes[0x0A] = {"LD A, (BC)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA,  0, nullptr)};
        Opcodes[0x0A] = {"LD A, (DE)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetDE, &CpuRegisters::SetA,  0, nullptr)};
        Opcodes[0x0A] = {"LD A, (HL+)", std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA,  1, &CpuRegisters::SetHL)};
        Opcodes[0x0A] = {"LD A, (HL=)", std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA, -1, &CpuRegisters::SetHL)};

        Opcodes[0x0B] = {"DEC BC", std::bind(decrementDoubleRegister, &CpuRegisters::GetBC, &CpuRegisters::SetBC)};
        Opcodes[0x1B] = {"DEC DE", std::bind(decrementDoubleRegister, &CpuRegisters::GetDE, &CpuRegisters::SetDE)};
        Opcodes[0x2B] = {"DEC HL", std::bind(decrementDoubleRegister, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x3B] = {"DEC SP", std::bind(decrementDoubleRegister, &CpuRegisters::GetSP, &CpuRegisters::SetSP)};

        Opcodes[0x0C] = {"INC C", std::bind(incrementRegister, &CpuRegisters::GetC, &CpuRegisters::SetC )};
        Opcodes[0x1C] = {"INC E", std::bind(incrementRegister, &CpuRegisters::GetE, &CpuRegisters::SetE )};
        Opcodes[0x2C] = {"INC L", std::bind(incrementRegister, &CpuRegisters::GetL, &CpuRegisters::SetL )};
        Opcodes[0x3C] = {"INC A", std::bind(incrementRegister, &CpuRegisters::GetA, &CpuRegisters::SetA )};

        Opcodes[0x0D] = {"DEC C", std::bind(decrementRegister, &CpuRegisters::GetC, &CpuRegisters::SetC )};
        Opcodes[0x1D] = {"DEC E", std::bind(decrementRegister, &CpuRegisters::GetE, &CpuRegisters::SetE )};
        Opcodes[0x2D] = {"DEC L", std::bind(decrementRegister, &CpuRegisters::GetL, &CpuRegisters::SetL )};
        Opcodes[0x3D] = {"DEC A", std::bind(decrementRegister, &CpuRegisters::GetA, &CpuRegisters::SetA )};

        Opcodes[0x0E] = { "LD C,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetC)};
        Opcodes[0x1E] = { "LD E,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetE)};
        Opcodes[0x2E] = { "LD L,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetL)};
        Opcodes[0x3E] = { "LD A,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetA)};

        Opcodes[0x0F] = { "RRCA", [this] { ExecutionSteps.push([this]{
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
            uint8_t val   = Registers.GetA();
            if(val == 0) { SetFlag(Flags::Zero); return; }
            uint8_t carry = (val & 1);
            val = (val >> 1) | (carry << 7);
            Registers.SetA(val);
            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            UnsetFlag(Flags::Zero);
        }); }};
        Opcodes[0x1F] = { "RRA",  [this] { ExecutionSteps.push([this]{
            UnsetFlag(Flags::Subtraction);
            UnsetFlag(Flags::HalfCarry);
            uint8_t val   = Registers.GetA();
            uint8_t carry = (val & 1);
            bool carrySet = IsFlagSet(Flags::Carry);
            val = (val >> 1) | ((carrySet ? 1 : 0) << 7);

            Registers.SetA(val);

            carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
            val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

        }); }};
        Opcodes[0x2F] = { "CPL",  [this] {Registers.Accumulator = ~(Registers.Accumulator); SetFlag(Flags::Subtraction); SetFlag(Flags::HalfCarry); }};
        Opcodes[0x3F] = { "CCF",  [this] { IsFlagSet(Flags::Carry) ? UnsetFlag(Flags::Carry) : SetFlag(Flags::Carry); }};

#pragma endregion

#pragma region 0x40 -> 0x6F

        Opcodes[0x40] = { "LD B, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x50] = { "LD D, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetD)};
        Opcodes[0x60] = { "LD H, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetH)};

        Opcodes[0x41] = { "LD B, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetB)};
        Opcodes[0x51] = { "LD D, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetD)};
        Opcodes[0x61] = { "LD H, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetH)};

        Opcodes[0x42] = { "LD B, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetB)};
        Opcodes[0x52] = { "LD D, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetD)};
        Opcodes[0x62] = { "LD H, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetH)};

        Opcodes[0x43] = { "LD B, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetB)};
        Opcodes[0x53] = { "LD D, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetD)};
        Opcodes[0x63] = { "LD H, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetH)};

        Opcodes[0x44] = { "LD B, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetB)};
        Opcodes[0x54] = { "LD D, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetD)};
        Opcodes[0x64] = { "LD H, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetH)};

        Opcodes[0x45] = { "LD B, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetB)};
        Opcodes[0x55] = { "LD D, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetD)};
        Opcodes[0x65] = { "LD H, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetH)};

        Opcodes[0x46] = {"LD B, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetB,  0, nullptr)};
        Opcodes[0x56] = {"LD D, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetD,  0, nullptr)};
        Opcodes[0x66] = {"LD H, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetH,  0, nullptr)};

        Opcodes[0x47] = { "LD B, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetB)};
        Opcodes[0x57] = { "LD D, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetD)};
        Opcodes[0x67] = { "LD H, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetH)};

        Opcodes[0x48] = { "LD B, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x58] = { "LD D, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetD)};
        Opcodes[0x68] = { "LD H, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetH)};

        Opcodes[0x49] = { "LD C, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetC)};
        Opcodes[0x59] = { "LD E, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetE)};
        Opcodes[0x69] = { "LD L, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetL)};

        Opcodes[0x4A] = { "LD C, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetC)};
        Opcodes[0x5A] = { "LD E, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetE)};
        Opcodes[0x6A] = { "LD L, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetL)};

        Opcodes[0x4B] = { "LD C, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetC)};
        Opcodes[0x5B] = { "LD E, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetE)};
        Opcodes[0x6B] = { "LD L, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetL)};

        Opcodes[0x4C] = { "LD C, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetC)};
        Opcodes[0x5C] = { "LD E, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetE)};
        Opcodes[0x6C] = { "LD L, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetL)};

        Opcodes[0x4D] = { "LD C, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetC)};
        Opcodes[0x5D] = { "LD E, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetE)};
        Opcodes[0x6D] = { "LD L, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetL)};

        Opcodes[0x4E] = {"LD C, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetC,  0, nullptr)};
        Opcodes[0x5E] = {"LD E, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetE,  0, nullptr)};
        Opcodes[0x6E] = {"LD L, (HL)",  std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetL,  0, nullptr)};

        Opcodes[0x4F] = { "LD C, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetC)};
        Opcodes[0x5F] = { "LD E, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetE)};
        Opcodes[0x6F] = { "LD L, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetL)};

#pragma endregion

#pragma  region 0x70 -> 0x75

        Opcodes[0x70] = {"LD (HL), B",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetB,  0, nullptr) };
        Opcodes[0x71] = {"LD (HL), C",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetC,  0, nullptr) };
        Opcodes[0x72] = {"LD (HL), D",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetD,  0, nullptr) };
        Opcodes[0x73] = {"LD (HL), E",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetE,  0, nullptr) };
        Opcodes[0x74] = {"LD (HL), H",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetH,  0, nullptr) };
        Opcodes[0x75] = {"LD (HL), L",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetL,  0, nullptr) };

#pragma  endregion

        //TODO: do the halt Opcode
        Opcodes[0x76] = { "HALT", [this]{} };

#pragma region 0x77 -> 0x7F

        Opcodes[0x77] = {"LD (HL), A",  std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA,  0, nullptr) };

        Opcodes[0x78] = { "LD A, B", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetA)};
        Opcodes[0x79] = { "LD A, C", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetA)};
        Opcodes[0x7A] = { "LD A, D", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetA)};
        Opcodes[0x7B] = { "LD A, E", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetA)};
        Opcodes[0x7C] = { "LD A, H", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetA)};
        Opcodes[0x7D] = { "LD A, L", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetA)};

        Opcodes[0x7E] = {"LD A, (HL)", std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetA,  0, nullptr)};

        Opcodes[0x7F] = { "LD A, A", std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetA)};

#pragma endregion

#pragma region 0x80 -> 0xBF

        Opcodes[0x80] = { "ADD A,B", std::bind(addRegisterToAccumulator, &CpuRegisters::GetB) };
        Opcodes[0x81] = { "ADD A,C", std::bind(addRegisterToAccumulator, &CpuRegisters::GetC) };
        Opcodes[0x82] = { "ADD A,D", std::bind(addRegisterToAccumulator, &CpuRegisters::GetD) };
        Opcodes[0x83] = { "ADD A,E", std::bind(addRegisterToAccumulator, &CpuRegisters::GetE) };
        Opcodes[0x84] = { "ADD A,H", std::bind(addRegisterToAccumulator, &CpuRegisters::GetH) };
        Opcodes[0x85] = { "ADD A,L", std::bind(addRegisterToAccumulator, &CpuRegisters::GetL) };

        Opcodes[0x86] = { "ADD A, (HL)", [this, HalfCarryHelper]{
            ExecutionSteps.push([this] {Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                int8_t reg = Read(Fetched);
                UnsetFlag(Flags::Subtraction);
                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t)reg + (uint16_t)Registers.Accumulator;
                Registers.SetA((uint8_t)result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF)  == 0 ? SetFlag(Flags::Zero)       : UnsetFlag(Flags::Zero);
            });
        }};

        Opcodes[0x87] = { "ADD A,A", std::bind(addRegisterToAccumulator, &CpuRegisters::GetA) };

        Opcodes[0x88] = { "ADC A,B", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetB) };
        Opcodes[0x89] = { "ADC A,C", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetC) };
        Opcodes[0x8A] = { "ADC A,D", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetD) };
        Opcodes[0x8B] = { "ADC A,E", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetE) };
        Opcodes[0x8C] = { "ADC A,H", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetH) };
        Opcodes[0x8D] = { "ADC A,L", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetL) };

        Opcodes[0x8E] = { "ADC A, (HL)", [this, HalfCarryHelper]{
            ExecutionSteps.push([this] {Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                int8_t reg = Read(Fetched);
                UnsetFlag(Flags::Subtraction);

                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                uint16_t result = (uint16_t)reg + (uint16_t)Registers.Accumulator + (uint16_t)carryVal;
                Registers.SetA((uint8_t)result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF)  == 0 ? SetFlag(Flags::Zero)       : UnsetFlag(Flags::Zero);
            });
        }};

        Opcodes[0x8F] = { "ADC A,A", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetA) };

#pragma endregion

    }
}