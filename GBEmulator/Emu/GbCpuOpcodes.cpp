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

#pragma endregion
    }
}