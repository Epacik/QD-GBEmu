//
// Created by epat on 17.07.2021.
//

#include "GbCpu.h"

namespace Emulator {
    namespace {
        typedef void     (CpuRegisters::*SetDoubleRegisterPtr)(uint16_t data);

        typedef uint16_t (CpuRegisters::*GetDoubleRegisterPtr)();

        typedef void     (CpuRegisters::*SetRegisterPtr)(uint8_t data);

        typedef uint8_t  (CpuRegisters::*GetRegisterPtr)();
    }

    //Defines actions that have to be performed for each main opcode
    //and stores them in an unordered map
    void GbCpu::SetOpcodes() {
        std::function<void(uint8_t, uint8_t)> HalfCarryHelper([this](uint8_t r1, uint8_t r2) {
            if ((((r1 & 0xf) + (r2 & 0xf)) & 0x10) == 0x10) {
                SetFlag(Flags::HalfCarry);
            } else {
                UnsetFlag(Flags::HalfCarry);
            }
        });
        std::function<void(uint16_t, uint16_t)> HalfCarryHelper16([this, HalfCarryHelper](uint16_t r1, uint16_t r2) {
            uint8_t r1u = (uint8_t) ((r1 & 0xFF00) >> 8);
            uint8_t r2u = (uint8_t) ((r2 & 0xFF00) >> 8);
            HalfCarryHelper(r1u, r2u);
        });

#pragma region  std::function templates ( or in other words, crimes against humanity )
        // I feel like that function and bindings below are crimes against humanity or something
        //buuut they work
        // for LD XX, d16
        std::function<void(SetDoubleRegisterPtr)> setDoubleRegisterToImmediateData
                ([this](SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this] { Fetched = (uint16_t) Fetch(); });
                    ExecutionSteps.push([this] { Fetched = Fetched | ((uint16_t) Fetch()) << 8; });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched); });
                });

        std::function<void(GetDoubleRegisterPtr, GetRegisterPtr, int,
                           SetDoubleRegisterPtr)> writeDataFromRegisterToAddress
                ([this](GetDoubleRegisterPtr getRegisterAddr, GetRegisterPtr getRegister, int incrementBy,
                        SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegisterAddr] { Fetched = std::invoke(getRegisterAddr, Registers); });
                    ExecutionSteps.push([this, getRegister, incrementBy, setRegister] {
                        Write(Fetched, std::invoke(getRegister, Registers));
                        if (incrementBy != 0) {
                            std::invoke(setRegister, Registers, Fetched + incrementBy);
                        }
                    });
                });

        std::function<void(GetDoubleRegisterPtr, SetDoubleRegisterPtr)> incrementDoubleRegister
                ([this](GetDoubleRegisterPtr getRegister, SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister] { Fetched = std::invoke(getRegister, Registers); });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched + 1); });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> incrementRegister
                ([this](GetRegisterPtr getRegister, SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, setRegister] {
                        uint8_t val = std::invoke(getRegister, Registers) + 1;
                        std::invoke(setRegister, Registers, val);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                        (val & 0b00001111) == 0 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                        UnsetFlag(Flags::Subtraction);
                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> decrementRegister
                ([this](GetRegisterPtr getRegister, SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, setRegister] {
                        uint8_t val = std::invoke(getRegister, Registers) - 1;
                        std::invoke(setRegister, Registers, val);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                        (val & 0b00001111) != 0b00001111 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                        SetFlag(Flags::Subtraction);
                    });
                });

        std::function<void(SetRegisterPtr)> loadDataFromAddressToRegisterImmediate
                ([this](SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this] { Fetched = Fetch(); });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched); });
                });

        std::function<void(GetDoubleRegisterPtr, GetDoubleRegisterPtr, SetDoubleRegisterPtr)> AddDoubleRegisters
                ([this, HalfCarryHelper16](GetDoubleRegisterPtr getRegister, GetDoubleRegisterPtr getRegister2,
                                           SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, getRegister2, setRegister, HalfCarryHelper16] {
                        uint32_t r1 = (uint32_t) std::invoke(getRegister, Registers);
                        uint32_t r2 = (uint32_t) std::invoke(getRegister2, Registers);
                        uint32_t val = r1 + r2;

                        val > 0xFFFF ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        HalfCarryHelper16(r1, r2);

                        Fetched = (uint16_t) (val & 0xFFFF);
                        std::invoke(setRegister, Registers, (Fetched & 0xFF) | (r1 & 0xFF00));
                    });
                    ExecutionSteps.push([this, setRegister] {
                        std::invoke(setRegister, Registers, Fetched);
                        UnsetFlag(Flags::Subtraction);
                    });
                });

        std::function<void(GetDoubleRegisterPtr, SetRegisterPtr, int,
                           SetDoubleRegisterPtr)> loadDataToRegisterByAddressInRegisterPair
                ([this](GetDoubleRegisterPtr getRegister, SetRegisterPtr setRegister, int incrementBy,
                        SetDoubleRegisterPtr setAddressReg) {
                    ExecutionSteps.push([this, getRegister, incrementBy, setAddressReg] {
                        Fetched = (uint16_t) std::invoke(getRegister, Registers);
                        if (incrementBy != 0)
                            std::invoke(setAddressReg, Registers, Fetched + incrementBy);
                    });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Read(Fetched)); });
                });

        std::function<void(GetDoubleRegisterPtr, SetDoubleRegisterPtr)> decrementDoubleRegister
                ([this](GetDoubleRegisterPtr getRegister, SetDoubleRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister] { Fetched = std::invoke(getRegister, Registers); });
                    ExecutionSteps.push([this, setRegister] { std::invoke(setRegister, Registers, Fetched - 1); });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr)> loadDataFromOneRegisterToAnother
                ([this](GetRegisterPtr getRegister, SetRegisterPtr setRegister) {
                    ExecutionSteps.push([this, getRegister, setRegister] {
                        std::invoke(setRegister, Registers, std::invoke(getRegister, Registers));
                    });
                });

        std::function<void(GetRegisterPtr)> addRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        UnsetFlag(Flags::Subtraction);
                        HalfCarryHelper(reg, Registers.Accumulator);
                        uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator;
                        Registers.SetA((uint8_t) result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> addRegisterAndCarryToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        UnsetFlag(Flags::Subtraction);
                        uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                        HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                        uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator + (uint16_t) carryVal;
                        Registers.SetA((uint8_t) result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> subtractRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        SetFlag(Flags::Subtraction);

                        HalfCarryHelper(reg, Registers.Accumulator);
                        uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                        Registers.SetA((uint8_t) result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> subtractRegisterAndCarryToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        SetFlag(Flags::Subtraction);
                        uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                        HalfCarryHelper(reg, Registers.Accumulator - carryVal);
                        uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg - carryVal;
                        Registers.SetA((uint8_t) result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> andRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                        Registers.SetA((uint8_t) result & 0xFF);

                        UnsetFlag(Flags::Subtraction);
                        SetFlag(Flags::HalfCarry);
                        UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> xorRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                        Registers.SetA((uint8_t) result ^ 0xFF);

                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);
                        UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> orRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                        Registers.SetA((uint8_t) result | 0xFF);

                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);
                        UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr)> compareRegisterToAccumulator
                ([this, HalfCarryHelper](GetRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister, HalfCarryHelper] {
                        uint8_t reg = std::invoke(getRegister, Registers);
                        SetFlag(Flags::Subtraction);

                        HalfCarryHelper(reg, Registers.Accumulator);
                        uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                        //Registers.SetA((uint8_t)result & 0xFF);

                        (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(bool)> returnFromSubroutineFlagged
                ([this](bool isFlagSet) {
                    ExecutionSteps.push([] {});
                    ExecutionSteps.push([] {});
                    if (isFlagSet) {
                        ExecutionSteps.push([this] { Fetched = (uint16_t) StackPop(); });
                        ExecutionSteps.push([this] { Fetched |= (uint16_t) StackPop() << 8; });
                        ExecutionSteps.push([this] { Registers.SetPC(Fetched); });
                    }
                });

        std::function<void(SetDoubleRegisterPtr)> popStackIntoDoubleRegister
                ([this](SetDoubleRegisterPtr setReg) {
                    ExecutionSteps.push([this] { Fetched = StackPop(); });
                    ExecutionSteps.push([this] { Fetched |= ((uint16_t) StackPop()) << 8; });
                    ExecutionSteps.push([this, setReg] { std::invoke(setReg, Registers, Fetched); });
                });

        std::function<void(bool)> jumpToImmediateAddress
                ([this](bool isFlagSet) {
                    ExecutionSteps.push([this] { Fetched = Fetch(); });
                    ExecutionSteps.push([this] { Fetched |= ((uint16_t) Fetch()) << 8; });
                    ExecutionSteps.push([] {});
                    if (isFlagSet) {
                        ExecutionSteps.push([this] { Registers.PC = Fetched; });
                    }
                });

        std::function<void()> illegal([this] { Bus->HangTheWholeEmulator(); });

        std::function<void(bool)> callImmediateAddress
                ([this](bool isFlagSet) {
                    ExecutionSteps.push([this] { Fetched = Fetch(); });
                    ExecutionSteps.push([this] { Fetched = ((uint16_t) Fetch()) << 8; });
                    ExecutionSteps.push([] {});
                    ExecutionSteps.push([] {});
                    if (isFlagSet) {
                        ExecutionSteps.push([this] { StackPush(((uint8_t) (Registers.PC & 0xFF00) >> 8)); });
                        ExecutionSteps.push([this] {
                            StackPush((uint8_t) (Registers.PC & 0xFF));
                            Registers.SetPC(Fetched);
                        });
                    }
                });

        std::function<void(GetDoubleRegisterPtr)> pushDoubleRegisterToStack
                ([this](GetDoubleRegisterPtr getRegister) {
                    ExecutionSteps.push([this, getRegister] { Fetched = std::invoke(getRegister, Registers); });
                    ExecutionSteps.push([this] { StackPush((uint8_t) ((Fetched & 0xFF00) >> 8)); });
                    ExecutionSteps.push([] {});
                    ExecutionSteps.push([this] { StackPush((uint8_t) (Fetched & 0xFF)); });
                });

        std::function<void(uint8_t)> reset
                ([this](uint8_t address) {
                    ExecutionSteps.push([this] { StackPush(((uint8_t) (Registers.PC & 0xFF00) >> 8)); });
                    ExecutionSteps.push([this] { StackPush((uint8_t) (Registers.PC & 0xFF)); });
                    ExecutionSteps.push([] {});
                    ExecutionSteps.push([this, address] { Registers.SetPC((uint16_t) (address & 0x00FF)); });
                });

#pragma endregion

#pragma region 0x00 - 0x3F

        Opcodes[0x00] = {"NOP", [this] { ExecutionSteps.push([] {}); }};
        Opcodes[0x10] = {"STOP", [this] {
            ExecutionSteps.push([this] {
                Fetch();
                Bus->Clock.Stop();
                //TODO: stop LCD
            });
        }};
        Opcodes[0x20] = {"JR NZ,", [this] {
            ExecutionSteps.push([this] { Fetched = (uint8_t) Fetch(); });
            ExecutionSteps.push([] {});
            if (!IsFlagSet(Flags::Zero)) {
                ExecutionSteps.push([this] { Registers.PC += (int8_t) Fetched; });
            }
        }};
        Opcodes[0x30] = {"JR NC,", [this] {
            ExecutionSteps.push([this] { Fetched = (uint8_t) Fetch(); });
            ExecutionSteps.push([] {});
            if (!IsFlagSet(Flags::Carry)) {
                ExecutionSteps.push([this] { Registers.PC += (int8_t) Fetched; });
            }
        }};

        Opcodes[0x01] = {"LD BC,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetBC)};
        Opcodes[0x11] = {"LD DE,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetDE)};
        Opcodes[0x21] = {"LD HL,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetHL)};
        Opcodes[0x31] = {"LD SP,", std::bind(setDoubleRegisterToImmediateData, &CpuRegisters::SetSP)};

        // TODO: do something about that additional text
//                [this] {
//                    return "";//Tools::StringConverters::GetHexString(Read(Registers.PC + 2)) +
//                    //Tools::StringConverters::GetHexString(Read(Registers.PC + 1));
//                }};

        Opcodes[0x02] = {"LD (BC), A",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetBC, &CpuRegisters::GetA, 0,
                                   nullptr)};
        Opcodes[0x12] = {"LD (DE), A",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetDE, &CpuRegisters::GetA, 0,
                                   nullptr)};
        Opcodes[0x22] = {"LD (HL+), A",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA, 1,
                                   &CpuRegisters::SetHL)};
        Opcodes[0x32] = {"LD (HL-), A",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA, -1,
                                   &CpuRegisters::SetHL)};
//        Opcodes[0x22] = {"LD (HL+), A", [this]  {
//            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched + 1); });
//            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
//        }};
//        Opcodes[0x32] = {"LD (HL-), A", [this]  {
//            ExecutionSteps.push([this]{ Fetched = Registers.GetHL(); Registers.SetHL(Fetched - 1); });
//            ExecutionSteps.push([this]{ Registers.SetA((uint8_t)Fetched); });
//        }};

        Opcodes[0x03] = {"INC BC", std::bind(incrementDoubleRegister, &CpuRegisters::GetBC, &CpuRegisters::SetBC)};
        Opcodes[0x13] = {"INC DE", std::bind(incrementDoubleRegister, &CpuRegisters::GetDE, &CpuRegisters::SetDE)};
        Opcodes[0x23] = {"INC HL", std::bind(incrementDoubleRegister, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x33] = {"INC SP", std::bind(incrementDoubleRegister, &CpuRegisters::GetSP, &CpuRegisters::SetSP)};

        Opcodes[0x04] = {"INC B", std::bind(incrementRegister, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x14] = {"INC D", std::bind(incrementRegister, &CpuRegisters::GetD, &CpuRegisters::SetD)};
        Opcodes[0x24] = {"INC H", std::bind(incrementRegister, &CpuRegisters::GetH, &CpuRegisters::SetH)};
        Opcodes[0x34] = {"INC (HL)", [this] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Fetched2 = Read(Fetched) + 1; });
            ExecutionSteps.push([this] {
                Write(Fetched, Fetched2);
                auto val = (uint8_t) Fetched2;
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                (val & 0b00001111) == 0 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Subtraction);
            });
        }};

        Opcodes[0x05] = {"DEC B", std::bind(decrementRegister, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x15] = {"DEC D", std::bind(decrementRegister, &CpuRegisters::GetD, &CpuRegisters::SetD)};
        Opcodes[0x25] = {"DEC H", std::bind(decrementRegister, &CpuRegisters::GetH, &CpuRegisters::SetH)};
        Opcodes[0x35] = {"DEC (HL)", [this] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Fetched2 = Read(Fetched) - 1; });
            ExecutionSteps.push([this] {
                Write(Fetched, Fetched2);
                auto val = (uint8_t) Fetched2;
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                (val & 0b00001111) != 0b1111 ? SetFlag(Flags::HalfCarry) : UnsetFlag(Flags::HalfCarry);
                SetFlag(Flags::Subtraction);
            });
        }};

        Opcodes[0x06] = {"LD B,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetB)};
        Opcodes[0x16] = {"LD D,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetD)};
        Opcodes[0x26] = {"LD H,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetH)};
        Opcodes[0x36] = {"LD (HL),", [this] {
            ExecutionSteps.push([this] { Fetched2 = Fetch(); });
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Write(Fetched, Fetched2); });
        }};

        Opcodes[0x07] = {"RLCA", [this] {
            ExecutionSteps.push([this] {
                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                uint8_t val = Registers.GetA();
                if (val == 0) {
                    SetFlag(Flags::Zero);
                    return;
                }
                uint8_t carry = (val & (1 << 7)) >> 7;
                val = (val << 1) | carry;
                Registers.SetA(val);
                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x17] = {"RLA", [this] {
            ExecutionSteps.push([this] {
                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                uint8_t val = Registers.GetA();
                uint8_t carry = (val & (1 << 7)) >> 7;
                bool carrySet = IsFlagSet(Flags::Carry);
                val = (val << 1) | (carrySet ? 1 : 0);

                Registers.SetA(val);

                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

            });
        }};
        Opcodes[0x27] = {"DAA", [this] {
            ExecutionSteps.push([this] {
                //TODO: Find something to test that thing correctly
                UnsetFlag(Flags::HalfCarry);
                uint8_t val = Registers.GetA();
                uint8_t cor = 0x00;
                bool setCarry = false;
                if (IsFlagSet(Flags::HalfCarry) || (!IsFlagSet(Flags::Subtraction) && (val & 0xf) > 9))
                    cor = 0x06;

                if (IsFlagSet(Flags::Carry) || (!IsFlagSet(Flags::Subtraction) && (val & 0xf) > 9)) {
                    cor |= 0x60;
                    setCarry = IsFlagSet(Flags::Carry);
                }

                IsFlagSet(Flags::Subtraction) ? val -= cor : val += cor;

                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                setCarry ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);

                Registers.SetA(val);
            });
        }};
        Opcodes[0x37] = {"SCF", [this] { ExecutionSteps.push([this] { SetFlag(Flags::Carry); }); }};

        Opcodes[0x08] = {"LD (a16), SP", [this] {
            ExecutionSteps.push([this] { Fetched = (uint16_t) Fetch(); });
            ExecutionSteps.push([this] { Fetched |= ((uint16_t) Fetch() << 8); });
            ExecutionSteps.push([this] { Fetched2 = Registers.GetSP(); });
            ExecutionSteps.push([this] { Fetched2 = (uint8_t) (Fetched2 & 0xFF); });
            ExecutionSteps.push([this] { Write(Fetched, (uint8_t) Fetched2); });
        }};
        Opcodes[0x18] = {"JR s8", [this] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Registers.PC += (int8_t) Fetched; });
        }};
        Opcodes[0x28] = {"JR Z, s8", [this] {
            ExecutionSteps.push([this] { Fetched = (uint8_t) Fetch(); });
            ExecutionSteps.push([] {});
            if (IsFlagSet(Flags::Zero)) {
                ExecutionSteps.push([this] { Registers.PC += (int8_t) Fetched; });
            }
        }};
        Opcodes[0x38] = {"JR C, s8", [this] {
            ExecutionSteps.push([this] { Fetched = (uint8_t) Fetch(); });
            ExecutionSteps.push([] {});
            if (IsFlagSet(Flags::Carry)) {
                ExecutionSteps.push([this] { Registers.PC += (int8_t) Fetched; });
            }
        }};

        Opcodes[0x09] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetBC, &CpuRegisters::GetHL,
                                                 &CpuRegisters::SetHL)};
        Opcodes[0x19] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetDE, &CpuRegisters::GetHL,
                                                 &CpuRegisters::SetHL)};
        Opcodes[0x29] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetHL, &CpuRegisters::GetHL,
                                                 &CpuRegisters::SetHL)};
        Opcodes[0x39] = {"ADD HL, BC", std::bind(AddDoubleRegisters, &CpuRegisters::GetSP, &CpuRegisters::GetHL,
                                                 &CpuRegisters::SetHL)};

        Opcodes[0x0A] = {"LD A, (BC)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA,
                                   0, nullptr)};
        Opcodes[0x0A] = {"LD A, (DE)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetDE, &CpuRegisters::SetA,
                                   0, nullptr)};
        Opcodes[0x0A] = {"LD A, (HL+)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA,
                                   1, &CpuRegisters::SetHL)};
        Opcodes[0x0A] = {"LD A, (HL=)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetBC, &CpuRegisters::SetA,
                                   -1, &CpuRegisters::SetHL)};

        Opcodes[0x0B] = {"DEC BC", std::bind(decrementDoubleRegister, &CpuRegisters::GetBC, &CpuRegisters::SetBC)};
        Opcodes[0x1B] = {"DEC DE", std::bind(decrementDoubleRegister, &CpuRegisters::GetDE, &CpuRegisters::SetDE)};
        Opcodes[0x2B] = {"DEC HL", std::bind(decrementDoubleRegister, &CpuRegisters::GetHL, &CpuRegisters::SetHL)};
        Opcodes[0x3B] = {"DEC SP", std::bind(decrementDoubleRegister, &CpuRegisters::GetSP, &CpuRegisters::SetSP)};

        Opcodes[0x0C] = {"INC C", std::bind(incrementRegister, &CpuRegisters::GetC, &CpuRegisters::SetC)};
        Opcodes[0x1C] = {"INC E", std::bind(incrementRegister, &CpuRegisters::GetE, &CpuRegisters::SetE)};
        Opcodes[0x2C] = {"INC L", std::bind(incrementRegister, &CpuRegisters::GetL, &CpuRegisters::SetL)};
        Opcodes[0x3C] = {"INC A", std::bind(incrementRegister, &CpuRegisters::GetA, &CpuRegisters::SetA)};

        Opcodes[0x0D] = {"DEC C", std::bind(decrementRegister, &CpuRegisters::GetC, &CpuRegisters::SetC)};
        Opcodes[0x1D] = {"DEC E", std::bind(decrementRegister, &CpuRegisters::GetE, &CpuRegisters::SetE)};
        Opcodes[0x2D] = {"DEC L", std::bind(decrementRegister, &CpuRegisters::GetL, &CpuRegisters::SetL)};
        Opcodes[0x3D] = {"DEC A", std::bind(decrementRegister, &CpuRegisters::GetA, &CpuRegisters::SetA)};

        Opcodes[0x0E] = {"LD C,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetC)};
        Opcodes[0x1E] = {"LD E,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetE)};
        Opcodes[0x2E] = {"LD L,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetL)};
        Opcodes[0x3E] = {"LD A,", std::bind(loadDataFromAddressToRegisterImmediate, &CpuRegisters::SetA)};

        Opcodes[0x0F] = {"RRCA", [this] {
            ExecutionSteps.push([this] {
                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                uint8_t val = Registers.GetA();
                if (val == 0) {
                    SetFlag(Flags::Zero);
                    return;
                }
                uint8_t carry = (val & 1);
                val = (val >> 1) | (carry << 7);
                Registers.SetA(val);
                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x1F] = {"RRA", [this] {
            ExecutionSteps.push([this] {
                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                uint8_t val = Registers.GetA();
                uint8_t carry = (val & 1);
                bool carrySet = IsFlagSet(Flags::Carry);
                val = (val >> 1) | ((carrySet ? 1 : 0) << 7);

                Registers.SetA(val);

                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

            });
        }};
        Opcodes[0x2F] = {"CPL", [this] {
            Registers.Accumulator = ~(Registers.Accumulator);
            SetFlag(Flags::Subtraction);
            SetFlag(Flags::HalfCarry);
        }};
        Opcodes[0x3F] = {"CCF", [this] { IsFlagSet(Flags::Carry) ? UnsetFlag(Flags::Carry) : SetFlag(Flags::Carry); }};

#pragma endregion

#pragma region 0x40 -> 0x6F

        Opcodes[0x40] = {"LD B, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x50] = {"LD D, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetD)};
        Opcodes[0x60] = {"LD H, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetH)};

        Opcodes[0x41] = {"LD B, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetB)};
        Opcodes[0x51] = {"LD D, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetD)};
        Opcodes[0x61] = {"LD H, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetH)};

        Opcodes[0x42] = {"LD B, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetB)};
        Opcodes[0x52] = {"LD D, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetD)};
        Opcodes[0x62] = {"LD H, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetH)};

        Opcodes[0x43] = {"LD B, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetB)};
        Opcodes[0x53] = {"LD D, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetD)};
        Opcodes[0x63] = {"LD H, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetH)};

        Opcodes[0x44] = {"LD B, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetB)};
        Opcodes[0x54] = {"LD D, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetD)};
        Opcodes[0x64] = {"LD H, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetH)};

        Opcodes[0x45] = {"LD B, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetB)};
        Opcodes[0x55] = {"LD D, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetD)};
        Opcodes[0x65] = {"LD H, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetH)};

        Opcodes[0x46] = {"LD B, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetB,
                                   0, nullptr)};
        Opcodes[0x56] = {"LD D, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetD,
                                   0, nullptr)};
        Opcodes[0x66] = {"LD H, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetH,
                                   0, nullptr)};

        Opcodes[0x47] = {"LD B, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetB)};
        Opcodes[0x57] = {"LD D, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetD)};
        Opcodes[0x67] = {"LD H, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetH)};

        Opcodes[0x48] = {"LD B, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetB)};
        Opcodes[0x58] = {"LD D, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetD)};
        Opcodes[0x68] = {"LD H, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetH)};

        Opcodes[0x49] = {"LD C, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetC)};
        Opcodes[0x59] = {"LD E, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetE)};
        Opcodes[0x69] = {"LD L, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetL)};

        Opcodes[0x4A] = {"LD C, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetC)};
        Opcodes[0x5A] = {"LD E, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetE)};
        Opcodes[0x6A] = {"LD L, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetL)};

        Opcodes[0x4B] = {"LD C, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetC)};
        Opcodes[0x5B] = {"LD E, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetE)};
        Opcodes[0x6B] = {"LD L, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetL)};

        Opcodes[0x4C] = {"LD C, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetC)};
        Opcodes[0x5C] = {"LD E, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetE)};
        Opcodes[0x6C] = {"LD L, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetL)};

        Opcodes[0x4D] = {"LD C, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetC)};
        Opcodes[0x5D] = {"LD E, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetE)};
        Opcodes[0x6D] = {"LD L, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetL)};

        Opcodes[0x4E] = {"LD C, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetC,
                                   0, nullptr)};
        Opcodes[0x5E] = {"LD E, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetE,
                                   0, nullptr)};
        Opcodes[0x6E] = {"LD L, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetL,
                                   0, nullptr)};

        Opcodes[0x4F] = {"LD C, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetC)};
        Opcodes[0x5F] = {"LD E, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetE)};
        Opcodes[0x6F] = {"LD L, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetL)};

#pragma endregion

#pragma  region 0x70 -> 0x75

        Opcodes[0x70] = {"LD (HL), B",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetB, 0,
                                   nullptr)};
        Opcodes[0x71] = {"LD (HL), C",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetC, 0,
                                   nullptr)};
        Opcodes[0x72] = {"LD (HL), D",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetD, 0,
                                   nullptr)};
        Opcodes[0x73] = {"LD (HL), E",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetE, 0,
                                   nullptr)};
        Opcodes[0x74] = {"LD (HL), H",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetH, 0,
                                   nullptr)};
        Opcodes[0x75] = {"LD (HL), L",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetL, 0,
                                   nullptr)};

#pragma  endregion

        //TODO: do the halt Opcode
        Opcodes[0x76] = {"HALT", [this] { Halt = true; }};

#pragma region 0x77 -> 0x7F

        Opcodes[0x77] = {"LD (HL), A",
                         std::bind(writeDataFromRegisterToAddress, &CpuRegisters::GetHL, &CpuRegisters::GetA, 0,
                                   nullptr)};

        Opcodes[0x78] = {"LD A, B",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetB, &CpuRegisters::SetA)};
        Opcodes[0x79] = {"LD A, C",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetC, &CpuRegisters::SetA)};
        Opcodes[0x7A] = {"LD A, D",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetD, &CpuRegisters::SetA)};
        Opcodes[0x7B] = {"LD A, E",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetE, &CpuRegisters::SetA)};
        Opcodes[0x7C] = {"LD A, H",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetH, &CpuRegisters::SetA)};
        Opcodes[0x7D] = {"LD A, L",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetL, &CpuRegisters::SetA)};

        Opcodes[0x7E] = {"LD A, (HL)",
                         std::bind(loadDataToRegisterByAddressInRegisterPair, &CpuRegisters::GetHL, &CpuRegisters::SetA,
                                   0, nullptr)};

        Opcodes[0x7F] = {"LD A, A",
                         std::bind(loadDataFromOneRegisterToAnother, &CpuRegisters::GetA, &CpuRegisters::SetA)};

#pragma endregion

#pragma region 0x80 -> 0xBF

        Opcodes[0x80] = {"ADD A,B", std::bind(addRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0x81] = {"ADD A,C", std::bind(addRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0x82] = {"ADD A,D", std::bind(addRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0x83] = {"ADD A,E", std::bind(addRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0x84] = {"ADD A,H", std::bind(addRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0x85] = {"ADD A,L", std::bind(addRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0x86] = {"ADD A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                UnsetFlag(Flags::Subtraction);
                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x87] = {"ADD A,A", std::bind(addRegisterToAccumulator, &CpuRegisters::GetA)};
        Opcodes[0x88] = {"ADC A,B", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0x89] = {"ADC A,C", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0x8A] = {"ADC A,D", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0x8B] = {"ADC A,E", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0x8C] = {"ADC A,H", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0x8D] = {"ADC A,L", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0x8E] = {"ADC A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                UnsetFlag(Flags::Subtraction);

                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator + (uint16_t) carryVal;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x8F] = {"ADC A,A", std::bind(addRegisterAndCarryToAccumulator, &CpuRegisters::GetA)};


        Opcodes[0x90] = {"SUB A,B", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0x91] = {"SUB A,C", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0x92] = {"SUB A,D", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0x93] = {"SUB A,E", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0x94] = {"SUB A,H", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0x95] = {"SUB A,L", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0x96] = {"SUB A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                SetFlag(Flags::Subtraction);
                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x97] = {"SUB A,A", std::bind(subtractRegisterToAccumulator, &CpuRegisters::GetA)};
        Opcodes[0x98] = {"SBC A,B", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0x99] = {"SBC A,C", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0x9A] = {"SBC A,D", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0x9B] = {"SBC A,E", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0x9C] = {"SBC A,H", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0x9D] = {"SBC A,L", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0x9E] = {"SBC A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                SetFlag(Flags::Subtraction);

                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator - carryVal);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg - (uint16_t) carryVal;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0x9F] = {"SBC A,A", std::bind(subtractRegisterAndCarryToAccumulator, &CpuRegisters::GetA)};

        Opcodes[0xA0] = {"AND A,B", std::bind(andRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0xA1] = {"AND A,C", std::bind(andRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0xA2] = {"AND A,D", std::bind(andRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0xA3] = {"AND A,E", std::bind(andRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0xA4] = {"AND A,H", std::bind(andRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0xA5] = {"AND A,L", std::bind(andRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0xA6] = {"AND A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result & 0xFF);

                UnsetFlag(Flags::Subtraction);
                SetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xA7] = {"AND A,A", std::bind(andRegisterToAccumulator, &CpuRegisters::GetA)};
        Opcodes[0xA8] = {"XOR A,B", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0xA9] = {"XOR A,C", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0xAA] = {"XOR A,D", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0xAB] = {"XOR A,E", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0xAC] = {"XOR A,H", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0xAD] = {"XOR A,L", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0xAE] = {"XOR A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result ^ 0xFF);

                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xAF] = {"XOR A,A", std::bind(xorRegisterToAccumulator, &CpuRegisters::GetA)};


        Opcodes[0xA0] = {"OR A,B", std::bind(orRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0xA1] = {"OR A,C", std::bind(orRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0xA2] = {"OR A,D", std::bind(orRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0xA3] = {"OR A,E", std::bind(orRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0xA4] = {"OR A,H", std::bind(orRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0xA5] = {"OR A,L", std::bind(orRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0xA6] = {"OR A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result & 0xFF);

                SetFlag(Flags::Subtraction);
                SetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xA7] = {"OR A,A", std::bind(orRegisterToAccumulator, &CpuRegisters::GetA)};
        Opcodes[0xA8] = {"CP A,B", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetB)};
        Opcodes[0xA9] = {"CP A,C", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetC)};
        Opcodes[0xAA] = {"CP A,D", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetD)};
        Opcodes[0xAB] = {"CP A,E", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetE)};
        Opcodes[0xAC] = {"CP A,H", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetH)};
        Opcodes[0xAD] = {"CP A,L", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetL)};
        Opcodes[0xAE] = {"CP A, (HL)", [this, HalfCarryHelper] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Read(Fetched);
                SetFlag(Flags::Subtraction);

                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                //Registers.SetA((uint8_t)result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xAF] = {"CP A,A", std::bind(compareRegisterToAccumulator, &CpuRegisters::GetA)};

#pragma endregion

#pragma region 0xC0 -> 0xFF

        Opcodes[0xC0] = {"RET NZ", std::bind(returnFromSubroutineFlagged, !IsFlagSet(Flags::Zero))};
        Opcodes[0xD0] = {"RET NC", std::bind(returnFromSubroutineFlagged, !IsFlagSet(Flags::Carry))};
        Opcodes[0xE0] = {"LD (a8), A", [this] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this] { Fetched |= 0xFF00; });
            ExecutionSteps.push([this] { Write(Fetched, Registers.GetA()); });
        }};
        Opcodes[0xF0] = {"LD A, (a8)", [this] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this] { Fetched |= 0xFF00; });
            ExecutionSteps.push([this] { Registers.SetA(Read(Fetched)); });
        }};

        Opcodes[0xC1] = {"POP BC", std::bind(popStackIntoDoubleRegister, &CpuRegisters::SetBC)};
        Opcodes[0xD1] = {"POP DE", std::bind(popStackIntoDoubleRegister, &CpuRegisters::SetDE)};
        Opcodes[0xE1] = {"POP HL", std::bind(popStackIntoDoubleRegister, &CpuRegisters::SetHL)};
        Opcodes[0xF1] = {"POP AF", std::bind(popStackIntoDoubleRegister, &CpuRegisters::SetAF)};

        Opcodes[0xC2] = {"JP NZ, a16", std::bind(jumpToImmediateAddress, !IsFlagSet(Flags::Zero))};
        Opcodes[0xD2] = {"JP NC, a16", std::bind(jumpToImmediateAddress, !IsFlagSet(Flags::Carry))};
        Opcodes[0xE2] = {"LD (C), A", [this] {
            ExecutionSteps.push([this] { Fetched = ((uint16_t) Registers.GetC()) | 0xFF00; });
            ExecutionSteps.push([this] { Write(Fetched, Registers.GetA()); });
        }};
        Opcodes[0xF2] = {"LD A, (C)", [this] {
            ExecutionSteps.push([this] { Fetched = ((uint16_t) Registers.GetC()) | 0xFF00; });
            ExecutionSteps.push([this] { Registers.SetA(Read(Fetched)); });
        }};

        Opcodes[0xC3] = {"JP NZ, a16", std::bind(jumpToImmediateAddress, true)};
        Opcodes[0xD3] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xE3] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xF3] = {"DI", [this] { ExecutionSteps.push([this] { InstructionsToResetIME = 1; }); }};

        Opcodes[0xC4] = {"CALL NZ, a16", std::bind(callImmediateAddress, !IsFlagSet(Flags::Zero))};
        Opcodes[0xD4] = {"CALL NC, a16", std::bind(callImmediateAddress, !IsFlagSet(Flags::Carry))};
        Opcodes[0xE4] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xF4] = {"ILLEGAL OPCODE", illegal};

        Opcodes[0xC5] = {"PUSH BC", std::bind(pushDoubleRegisterToStack, &CpuRegisters::GetBC)};
        Opcodes[0xD5] = {"PUSH DE", std::bind(pushDoubleRegisterToStack, &CpuRegisters::GetDE)};
        Opcodes[0xE5] = {"PUSH HL", std::bind(pushDoubleRegisterToStack, &CpuRegisters::GetHL)};
        Opcodes[0xF5] = {"PUSH AF", std::bind(pushDoubleRegisterToStack, &CpuRegisters::GetAF)};

        Opcodes[0xC6] = {"ADD A, d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();//std::invoke(getRegister, Registers);
                UnsetFlag(Flags::Subtraction);
                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator + (uint16_t) carryVal;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xD6] = {"SUB d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                SetFlag(Flags::Subtraction);

                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xE6] = {"AND d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result & 0xFF);

                UnsetFlag(Flags::Subtraction);
                SetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0XF6] = {"OR d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result | 0xFF);

                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};

        Opcodes[0xC7] = {"RST 00h", std::bind(reset, 0x00)};
        Opcodes[0xD7] = {"RST 10h", std::bind(reset, 0x10)};
        Opcodes[0xE7] = {"RST 20h", std::bind(reset, 0x20)};
        Opcodes[0xF7] = {"RST 30h", std::bind(reset, 0x30)};

        Opcodes[0xC8] = {"RET Z", std::bind(returnFromSubroutineFlagged, IsFlagSet(Flags::Zero))};
        Opcodes[0xD8] = {"RET C", std::bind(returnFromSubroutineFlagged, IsFlagSet(Flags::Carry))};
        Opcodes[0xE8] = {"ADD SP, s8", [this, HalfCarryHelper16] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this, HalfCarryHelper16] {
                HalfCarryHelper16(((int16_t) Fetched), Registers.GetSP());
                UnsetFlag(Flags::Zero);
                UnsetFlag(Flags::Subtraction);
                (((int32_t) Fetched) + (uint32_t) Registers.GetSP()) ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                Fetched = ((int16_t) Fetched) + Registers.GetSP();
            });
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Registers.SetSP(Fetched); });
        }};
        Opcodes[0xF8] = {"LD HL, SP+s8", [this, HalfCarryHelper16] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this, HalfCarryHelper16] {
                HalfCarryHelper16(((int16_t) Fetched), Registers.GetSP());
                UnsetFlag(Flags::Zero);
                UnsetFlag(Flags::Subtraction);
                (((int32_t) Fetched) + (uint32_t) Registers.GetSP()) ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                Fetched = ((int16_t) Fetched) + Registers.GetSP();
            });
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Registers.SetHL(Fetched); });
        }};

        Opcodes[0xC9] = {"RET", [this] {
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Fetched = (uint16_t) StackPop(); });
            ExecutionSteps.push([this] { Fetched |= (uint16_t) StackPop() << 8; });
            ExecutionSteps.push([this] { Registers.SetPC(Fetched); });
        }};
        Opcodes[0xD9] = {"RETI", [this] {
            ExecutionSteps.push([this] { Bus->InterruptEnableRegister = IMEBack; });
            ExecutionSteps.push([this] { Fetched = (uint16_t) StackPop(); });
            ExecutionSteps.push([this] { Fetched |= (uint16_t) StackPop() << 8; });
            ExecutionSteps.push([this] { Registers.SetPC(Fetched); });
        }};
        Opcodes[0xE9] = {"JP HL", [this] {
            ExecutionSteps.push([this] { Registers.SetPC(Registers.GetHL()); });
        }};
        Opcodes[0xF9] = {"LD SP, HL", [this] {
            ExecutionSteps.push([this] { Fetched = Registers.GetHL(); });
            ExecutionSteps.push([this] { Registers.SetSP(Fetched); });
        }};

        Opcodes[0xCA] = {"JP Z, a16", std::bind(jumpToImmediateAddress, IsFlagSet(Flags::Zero))};
        Opcodes[0xDA] = {"JP C, a16", std::bind(jumpToImmediateAddress, IsFlagSet(Flags::Carry))};
        Opcodes[0xEA] = {"LD (a16), A", [this] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this] { Fetched = (((uint16_t) Fetch()) << 8); });
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Write(Fetched, Registers.GetA()); });
        }};
        Opcodes[0xFA] = {"LD A, (a16)", [this] {
            ExecutionSteps.push([this] { Fetched = Fetch(); });
            ExecutionSteps.push([this] { Fetched = (((uint16_t) Fetch()) << 8); });
            ExecutionSteps.push([] {});
            ExecutionSteps.push([this] { Registers.SetA(Read(Fetched)); });
        }};

        Opcodes[0xCB] = {"", [this] {
            auto f = Fetch();
            OpcodesEx[f].Exec();
        }};
        Opcodes[0xDB] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xEB] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xFB] = {"EI", [this] { ExecutionSteps.push([this] { InstructionsToSetIME = 1; }); }};

        Opcodes[0xCC] = {"CALL Z, a16", std::bind(callImmediateAddress, IsFlagSet(Flags::Zero))};
        Opcodes[0xDC] = {"CALL C, a16", std::bind(callImmediateAddress, IsFlagSet(Flags::Carry))};
        Opcodes[0xEC] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xFC] = {"ILLEGAL OPCODE", illegal};

        Opcodes[0xCD] = {"CALL a16", std::bind(callImmediateAddress, true)};
        Opcodes[0xDD] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xED] = {"ILLEGAL OPCODE", illegal};
        Opcodes[0xFD] = {"ILLEGAL OPCODE", illegal};

        Opcodes[0xCE] = {"ADC A, d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                UnsetFlag(Flags::Subtraction);
                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator + carryVal);
                uint16_t result = (uint16_t) reg + (uint16_t) Registers.Accumulator + (uint16_t) carryVal;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xDE] = {"SBC A, d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                SetFlag(Flags::Subtraction);
                uint8_t carryVal = IsFlagSet(Flags::Carry) ? 1 : 0;
                HalfCarryHelper(reg, Registers.Accumulator - carryVal);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg - carryVal;
                Registers.SetA((uint8_t) result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xEE] = {"XOR d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                uint16_t result = (uint16_t) Registers.Accumulator & (uint16_t) reg;
                Registers.SetA((uint8_t) result ^ 0xFF);

                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);
                UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};
        Opcodes[0xFE] = {"CP d8", [this, HalfCarryHelper] {
            ExecutionSteps.push([this, HalfCarryHelper] {
                uint8_t reg = Fetch();
                SetFlag(Flags::Subtraction);

                HalfCarryHelper(reg, Registers.Accumulator);
                uint16_t result = (uint16_t) Registers.Accumulator - (uint16_t) reg;
                //Registers.SetA((uint8_t)result & 0xFF);

                (result & 0xFF00) > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                (result & 0xFF) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        }};

        Opcodes[0xC7] = {"RST 08h", std::bind(reset, 0x08)};
        Opcodes[0xD7] = {"RST 18h", std::bind(reset, 0x18)};
        Opcodes[0xE7] = {"RST 28h", std::bind(reset, 0x28)};
        Opcodes[0xF7] = {"RST 38h", std::bind(reset, 0x38)};

#pragma endregion

    }

    //Defines actions that have to be performed for each opcode starting with 0xCB
    //and stores them in an unordered map
    void GbCpu::SetExtendedOpcodes() {

#pragma region  std::function templates ( or in other words, crimes against humanity )

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> rotateRegisterLeft
        ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false){
            ExecutionSteps.push([this, get]{
                uint8_t val = std::invoke(get, Registers);
                Fetched = val;
            });

            if(waste2Clocks){
                ExecutionSteps.push([]{});
                ExecutionSteps.push([]{});
            }

            ExecutionSteps.push([this, set]{
                auto val = (uint8_t)Fetched;

                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);

                uint8_t carry = (val & (1 << 7)) >> 7;
                val = (val << 1) | carry;
                std::invoke(set, Registers, val);
                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
            });
        });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> rotateRegisterRight
        ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false) {
            ExecutionSteps.push([this, get]{
                uint8_t val = std::invoke(get, Registers);
                Fetched = val;
            });

            if(waste2Clocks){
                ExecutionSteps.push([]{});
                ExecutionSteps.push([]{});
            }

            ExecutionSteps.push([this, set]{
                auto val = (uint8_t)Fetched;



                UnsetFlag(Flags::Subtraction);
                UnsetFlag(Flags::HalfCarry);

                uint8_t carry = (val & 1);
                val = (val >> 1) | (carry << 7);
                std::invoke(set, Registers, val);
                carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

            });
        });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> rotateRegisterLeftTroughCarry
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false){
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;
                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);


                        uint8_t carry = (val & (1 << 7)) >> 7;
                        bool carrySet = IsFlagSet(Flags::Carry);
                        val = (val << 1) | (carrySet ? 1 : 0);

                        std::invoke(set, Registers, val);

                        carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> rotateRegisterRightTroughCarry
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false) {
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;
                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);


                        uint8_t carry = (val & 1);
                        bool carrySet = IsFlagSet(Flags::Carry);
                        val = (val >> 1);

                        std::invoke(set, Registers, val);

                        carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> shiftRegisterLeft
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false){
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;

                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);

                        uint8_t carry = (val & (1 << 7)) >> 7;
                        val = (val << 1);
                        std::invoke(set, Registers, val);
                        carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> shiftRegisterRightNoBit7Change
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false) {
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;
                        uint8_t b7 = val & 1 << 7;
                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);

                        uint8_t carry = (val & 1);
                        val = (val >> 1) | b7;
                        std::invoke(set, Registers, val);
                        carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> swapNybblesOfARegister
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false) {
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;
                        uint8_t b7 = val & 1 << 7;
                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);
                        UnsetFlag(Flags::Carry);

                        uint8_t low  = 0xF  & val;
                        uint8_t high = 0xF0 & val;

                        val = (low << 4) | (high >> 4);
                        std::invoke(set, Registers, val);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

                    });
                });

        std::function<void(GetRegisterPtr, SetRegisterPtr, bool)> shiftRegisterRight
                ([this](GetRegisterPtr get, SetRegisterPtr set, bool waste2Clocks = false) {
                    ExecutionSteps.push([this, get]{
                        uint8_t val = std::invoke(get, Registers);
                        Fetched = val;
                    });

                    if(waste2Clocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }

                    ExecutionSteps.push([this, set]{
                        auto val = (uint8_t)Fetched;
                        uint8_t b7 = val & 1 << 7;
                        UnsetFlag(Flags::Subtraction);
                        UnsetFlag(Flags::HalfCarry);

                        uint8_t carry = (val & 1);
                        val = (val >> 1);
                        std::invoke(set, Registers, val);
                        carry > 0 ? SetFlag(Flags::Carry) : UnsetFlag(Flags::Carry);
                        val == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);

                    });
                });

        std::function<void(uint8_t, GetRegisterPtr, bool)> testBitOfRegister
            ([this](uint8_t bit, GetRegisterPtr get, bool wasteClock){
                ExecutionSteps.push([this, get]{
                    uint8_t val = std::invoke(get, Registers);
                    Fetched = val;
                });

                if(wasteClock){
                    ExecutionSteps.push([]{});
                }
                ExecutionSteps.push([this, bit]{
                    (((uint8_t)Fetched) & bit) == 0 ? SetFlag(Flags::Zero) : UnsetFlag(Flags::Zero);
                    UnsetFlag(Flags::Subtraction);
                    SetFlag(Flags::HalfCarry);
                });
            });

        std::function<void(uint8_t, GetRegisterPtr, SetRegisterPtr, bool)> resetBitOfRegister
                ([this](uint8_t bit, GetRegisterPtr get, SetRegisterPtr set, bool wasteClocks){
                    ExecutionSteps.push([this, get]{

                        Fetched = (uint8_t)std::invoke(get, Registers);
                    });

                    if(wasteClocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }
                    ExecutionSteps.push([this, bit, set]{
                        std::invoke(set, Registers, ~bit & ((uint8_t)Fetched));
                    });
                });

        std::function<void(uint8_t, GetRegisterPtr, SetRegisterPtr, bool)> setBitOfRegister
                ([this](uint8_t bit, GetRegisterPtr get, SetRegisterPtr set, bool wasteClocks){
                    ExecutionSteps.push([this, get]{

                        Fetched = (uint8_t)std::invoke(get, Registers);
                    });

                    if(wasteClocks){
                        ExecutionSteps.push([]{});
                        ExecutionSteps.push([]{});
                    }
                    ExecutionSteps.push([this, bit, set]{
                        std::invoke(set, Registers, bit | ((uint8_t)Fetched));
                    });
                });
#pragma endregion

#pragma region 0x00 -> 0x0F

        OpcodesEx[0x00] = {"RLC B", std::bind(rotateRegisterLeft, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x01] = {"RLC C", std::bind(rotateRegisterLeft, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x02] = {"RLC D", std::bind(rotateRegisterLeft, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x03] = {"RLC E", std::bind(rotateRegisterLeft, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x04] = {"RLC H", std::bind(rotateRegisterLeft, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x05] = {"RLC L", std::bind(rotateRegisterLeft, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x06] = {"RLC (HL)", std::bind(rotateRegisterLeft, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x07] = {"RLC A", std::bind(rotateRegisterLeft, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0x08] = {"RRC B", std::bind(rotateRegisterRight, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x09] = {"RRC C", std::bind(rotateRegisterRight, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x0A] = {"RRC D", std::bind(rotateRegisterRight, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x0B] = {"RRC E", std::bind(rotateRegisterRight, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x0C] = {"RRC H", std::bind(rotateRegisterRight, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x0D] = {"RRC L", std::bind(rotateRegisterRight, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x0E] = {"RRC (HL)", std::bind(rotateRegisterRight, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x0F] = {"RRC A", std::bind(rotateRegisterRight, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

#pragma endregion
#pragma  region 0x10 -> 0x1F

        OpcodesEx[0x10] = {"RL B", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x11] = {"RL C", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x12] = {"RL D", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x13] = {"RL E", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x14] = {"RL H", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x15] = {"RL L", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x16] = {"RL (HL)", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x17] = {"RL A", std::bind(rotateRegisterLeftTroughCarry, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0x18] = {"RR B", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x19] = {"RR C", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x1A] = {"RR D", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x1B] = {"RR E", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x1C] = {"RR H", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x1D] = {"RR L", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x1E] = {"RR (HL)", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x1F] = {"RR A", std::bind(rotateRegisterRightTroughCarry, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

#pragma endregion
#pragma  region 0x20 -> 0x2F

        OpcodesEx[0x20] = {"SLA B", std::bind(shiftRegisterLeft, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x21] = {"SLA C", std::bind(shiftRegisterLeft, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x22] = {"SLA D", std::bind(shiftRegisterLeft, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x23] = {"SLA E", std::bind(shiftRegisterLeft, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x24] = {"SLA H", std::bind(shiftRegisterLeft, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x25] = {"SLA L", std::bind(shiftRegisterLeft, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x26] = {"SLA (HL)", std::bind(shiftRegisterLeft, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x27] = {"SLA A", std::bind(shiftRegisterLeft, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0x28] = {"SRA B", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x29] = {"SRA C", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x2A] = {"SRA D", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x2B] = {"SRA E", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x2C] = {"SRA H", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x2D] = {"SRA L", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x2E] = {"SRA (HL)", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x2F] = {"SRA A", std::bind(shiftRegisterRightNoBit7Change, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

#pragma endregion
#pragma  region 0x30 -> 0x3F

        OpcodesEx[0x30] = {"SWAP B", std::bind(swapNybblesOfARegister, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x31] = {"SWAP C", std::bind(swapNybblesOfARegister, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x32] = {"SWAP D", std::bind(swapNybblesOfARegister, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x33] = {"SWAP E", std::bind(swapNybblesOfARegister, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x34] = {"SWAP H", std::bind(swapNybblesOfARegister, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x35] = {"SWAP L", std::bind(swapNybblesOfARegister, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x36] = {"SWAP (HL)", std::bind(swapNybblesOfARegister, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x37] = {"SWAP A", std::bind(swapNybblesOfARegister, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0x38] = {"SRL B", std::bind(shiftRegisterRight, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x39] = {"SRL C", std::bind(shiftRegisterRight, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x3A] = {"SRL D", std::bind(shiftRegisterRight, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x3B] = {"SRL E", std::bind(shiftRegisterRight, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x3C] = {"SRL H", std::bind(shiftRegisterRight, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x3D] = {"SRL L", std::bind(shiftRegisterRight, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x3E] = {"SRL (HL)", std::bind(shiftRegisterRight, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x3F] = {"SRL A", std::bind(shiftRegisterRight, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

#pragma endregion
#pragma region 0x40 -> 0x7F

        OpcodesEx[0x40] = {"BIT 0, B", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetB, false)};
        OpcodesEx[0x41] = {"BIT 0, C", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetC, false)};
        OpcodesEx[0x42] = {"BIT 0, D", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetD, false)};
        OpcodesEx[0x43] = {"BIT 0, E", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetE, false)};
        OpcodesEx[0x44] = {"BIT 0, H", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetH, false)};
        OpcodesEx[0x45] = {"BIT 0, L", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetL, false)};
        OpcodesEx[0x46] = {"BIT 0, (HL)", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x47] = {"BIT 0, A", std::bind(testBitOfRegister, 0b1 << 0, &CpuRegisters::GetA, false)};

        OpcodesEx[0x48] = {"BIT 1, B", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetB, false)};
        OpcodesEx[0x49] = {"BIT 1, C", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetC, false)};
        OpcodesEx[0x4A] = {"BIT 1, D", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetD, false)};
        OpcodesEx[0x4B] = {"BIT 1, E", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetE, false)};
        OpcodesEx[0x4C] = {"BIT 1, H", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetH, false)};
        OpcodesEx[0x4D] = {"BIT 1, L", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetL, false)};
        OpcodesEx[0x4E] = {"BIT 1, (HL)", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x4F] = {"BIT 1, A", std::bind(testBitOfRegister, 0b1 << 1, &CpuRegisters::GetA, false)};



        OpcodesEx[0x50] = {"BIT 2, B", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetB, false)};
        OpcodesEx[0x51] = {"BIT 2, C", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetC, false)};
        OpcodesEx[0x52] = {"BIT 2, D", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetD, false)};
        OpcodesEx[0x53] = {"BIT 2, E", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetE, false)};
        OpcodesEx[0x54] = {"BIT 2, H", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetH, false)};
        OpcodesEx[0x55] = {"BIT 2, L", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetL, false)};
        OpcodesEx[0x56] = {"BIT 2, (HL)", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x57] = {"BIT 2, A", std::bind(testBitOfRegister, 0b1 << 2, &CpuRegisters::GetA, false)};

        OpcodesEx[0x58] = {"BIT 3, B", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetB, false)};
        OpcodesEx[0x59] = {"BIT 3, C", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetC, false)};
        OpcodesEx[0x5A] = {"BIT 3, D", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetD, false)};
        OpcodesEx[0x5B] = {"BIT 3, E", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetE, false)};
        OpcodesEx[0x5C] = {"BIT 3, H", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetH, false)};
        OpcodesEx[0x5D] = {"BIT 3, L", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetL, false)};
        OpcodesEx[0x5E] = {"BIT 3, (HL)", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x5F] = {"BIT 3, A", std::bind(testBitOfRegister, 0b1 << 3, &CpuRegisters::GetA, false)};


        OpcodesEx[0x60] = {"BIT 4, B", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetB, false)};
        OpcodesEx[0x61] = {"BIT 4, C", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetC, false)};
        OpcodesEx[0x62] = {"BIT 4, D", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetD, false)};
        OpcodesEx[0x63] = {"BIT 4, E", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetE, false)};
        OpcodesEx[0x64] = {"BIT 4, H", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetH, false)};
        OpcodesEx[0x65] = {"BIT 4, L", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetL, false)};
        OpcodesEx[0x66] = {"BIT 4, (HL)", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x67] = {"BIT 4, A", std::bind(testBitOfRegister, 0b1 << 4, &CpuRegisters::GetA, false)};

        OpcodesEx[0x68] = {"BIT 5, B", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetB, false)};
        OpcodesEx[0x69] = {"BIT 5, C", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetC, false)};
        OpcodesEx[0x6A] = {"BIT 5, D", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetD, false)};
        OpcodesEx[0x6B] = {"BIT 5, E", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetE, false)};
        OpcodesEx[0x6C] = {"BIT 5, H", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetH, false)};
        OpcodesEx[0x6D] = {"BIT 5, L", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetL, false)};
        OpcodesEx[0x6E] = {"BIT 5, (HL)", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x6F] = {"BIT 5, A", std::bind(testBitOfRegister, 0b1 << 5, &CpuRegisters::GetA, false)};


        OpcodesEx[0x70] = {"BIT 6, B", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetB, false)};
        OpcodesEx[0x71] = {"BIT 6, C", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetC, false)};
        OpcodesEx[0x72] = {"BIT 6, D", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetD, false)};
        OpcodesEx[0x73] = {"BIT 6, E", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetE, false)};
        OpcodesEx[0x74] = {"BIT 6, H", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetH, false)};
        OpcodesEx[0x75] = {"BIT 6, L", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetL, false)};
        OpcodesEx[0x76] = {"BIT 6, (HL)", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x77] = {"BIT 6, A", std::bind(testBitOfRegister, 0b1 << 6, &CpuRegisters::GetA, false)};

        OpcodesEx[0x78] = {"BIT 7, B", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetB, false)};
        OpcodesEx[0x79] = {"BIT 7, C", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetC, false)};
        OpcodesEx[0x7A] = {"BIT 7, D", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetD, false)};
        OpcodesEx[0x7B] = {"BIT 7, E", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetE, false)};
        OpcodesEx[0x7C] = {"BIT 7, H", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetH, false)};
        OpcodesEx[0x7D] = {"BIT 7, L", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetL, false)};
        OpcodesEx[0x7E] = {"BIT 7, (HL)", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetValueAtAddressInHL, true)};
        OpcodesEx[0x7F] = {"BIT 7, A", std::bind(testBitOfRegister, 0b1 << 7, &CpuRegisters::GetA, false)};

#pragma endregion
#pragma region 0x80 -> 0xBF

        OpcodesEx[0x80] = {"RES 0, B",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x81] = {"RES 0, C",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x82] = {"RES 0, D",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x83] = {"RES 0, E",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x84] = {"RES 0, H",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x85] = {"RES 0, L",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x86] = {"RES 0, (HL)", std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x87] = {"RES 0, A",    std::bind(resetBitOfRegister, 0x1 << 0, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0x88] = {"RES 1, B",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x89] = {"RES 1, C",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x8A] = {"RES 1, D",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x8B] = {"RES 1, E",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x8C] = {"RES 1, H",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x8D] = {"RES 1, L",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x8E] = {"RES 1, (HL)", std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x8F] = {"RES 1, A",    std::bind(resetBitOfRegister, 0x1 << 1, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0x90] = {"RES 2, B",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x91] = {"RES 2, C",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x92] = {"RES 2, D",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x93] = {"RES 2, E",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x94] = {"RES 2, H",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x95] = {"RES 2, L",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x96] = {"RES 2, (HL)", std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x97] = {"RES 2, A",    std::bind(resetBitOfRegister, 0x1 << 2, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0x98] = {"RES 3, B",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0x99] = {"RES 3, C",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0x9A] = {"RES 3, D",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0x9B] = {"RES 3, E",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0x9C] = {"RES 3, H",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0x9D] = {"RES 3, L",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0x9E] = {"RES 3, (HL)", std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0x9F] = {"RES 3, A",    std::bind(resetBitOfRegister, 0x1 << 3, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0xA0] = {"RES 4, B",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xA1] = {"RES 4, C",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xA2] = {"RES 4, D",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xA3] = {"RES 4, E",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xA4] = {"RES 4, H",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xA5] = {"RES 4, L",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xA6] = {"RES 4, (HL)", std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xA7] = {"RES 4, A",    std::bind(resetBitOfRegister, 0x1 << 4, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xA8] = {"RES 5, B",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xA9] = {"RES 5, C",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xAA] = {"RES 5, D",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xAB] = {"RES 5, E",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xAC] = {"RES 5, H",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xAD] = {"RES 5, L",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xAE] = {"RES 5, (HL)", std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xAF] = {"RES 5, A",    std::bind(resetBitOfRegister, 0x1 << 5, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0xA0] = {"RES 6, B",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xA1] = {"RES 6, C",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xA2] = {"RES 6, D",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xA3] = {"RES 6, E",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xA4] = {"RES 6, H",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xA5] = {"RES 6, L",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xA6] = {"RES 6, (HL)", std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xA7] = {"RES 6, A",    std::bind(resetBitOfRegister, 0x1 << 6, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xA8] = {"RES 7, B",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xA9] = {"RES 7, C",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xAA] = {"RES 7, D",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xAB] = {"RES 7, E",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xAC] = {"RES 7, H",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xAD] = {"RES 7, L",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xAE] = {"RES 7, (HL)", std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xAF] = {"RES 7, A",    std::bind(resetBitOfRegister, 0x1 << 7, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
#pragma endregion
#pragma region 0xC0 -> 0xFF

        OpcodesEx[0xC0] = {"SET 0, B",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xC1] = {"SET 0, C",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xC2] = {"SET 0, D",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xC3] = {"SET 0, E",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xC4] = {"SET 0, H",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xC5] = {"SET 0, L",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xC6] = {"SET 0, (HL)", std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xC7] = {"SET 0, A",    std::bind(setBitOfRegister, 0x1 << 0, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xC8] = {"SET 1, B",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xC9] = {"SET 1, C",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xCA] = {"SET 1, D",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xCB] = {"SET 1, E",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xCC] = {"SET 1, H",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xCD] = {"SET 1, L",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xCE] = {"SET 1, (HL)", std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xCF] = {"SET 1, A",    std::bind(setBitOfRegister, 0x1 << 1, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0xD0] = {"SET 2, B",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xD1] = {"SET 2, C",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xD2] = {"SET 2, D",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xD3] = {"SET 2, E",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xD4] = {"SET 2, H",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xD5] = {"SET 2, L",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xD6] = {"SET 2, (HL)", std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xD7] = {"SET 2, A",    std::bind(setBitOfRegister, 0x1 << 2, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xD8] = {"SET 3, B",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xD9] = {"SET 3, C",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xDA] = {"SET 3, D",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xDB] = {"SET 3, E",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xDC] = {"SET 3, H",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xDD] = {"SET 3, L",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xDE] = {"SET 3, (HL)", std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xDF] = {"SET 3, A",    std::bind(setBitOfRegister, 0x1 << 3, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0xE0] = {"SET 4, B",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xE1] = {"SET 4, C",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xE2] = {"SET 4, D",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xE3] = {"SET 4, E",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xE4] = {"SET 4, H",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xE5] = {"SET 4, L",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xE6] = {"SET 4, (HL)", std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xE7] = {"SET 4, A",    std::bind(setBitOfRegister, 0x1 << 4, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xE8] = {"SET 5, B",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xE9] = {"SET 5, C",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xEA] = {"SET 5, D",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xEB] = {"SET 5, E",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xEC] = {"SET 5, H",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xED] = {"SET 5, L",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xEE] = {"SET 5, (HL)", std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xEF] = {"SET 5, A",    std::bind(setBitOfRegister, 0x1 << 5, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};

        OpcodesEx[0xF0] = {"SET 6, B",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xF1] = {"SET 6, C",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xF2] = {"SET 6, D",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xF3] = {"SET 6, E",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xF4] = {"SET 6, H",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xF5] = {"SET 6, L",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xF6] = {"SET 6, (HL)", std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xF7] = {"SET 6, A",    std::bind(setBitOfRegister, 0x1 << 6, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
        OpcodesEx[0xF8] = {"SET 7, B",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetB, &CpuRegisters::SetB, false)};
        OpcodesEx[0xF9] = {"SET 7, C",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetC, &CpuRegisters::SetC, false)};
        OpcodesEx[0xFA] = {"SET 7, D",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetD, &CpuRegisters::SetD, false)};
        OpcodesEx[0xFB] = {"SET 7, E",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetE, &CpuRegisters::SetE, false)};
        OpcodesEx[0xFC] = {"SET 7, H",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetH, &CpuRegisters::SetH, false)};
        OpcodesEx[0xFD] = {"SET 7, L",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetL, &CpuRegisters::SetL, false)};
        OpcodesEx[0xFE] = {"SET 7, (HL)", std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetValueAtAddressInHL, &CpuRegisters::SetValueAtAddtesInHL, true)};
        OpcodesEx[0xFF] = {"SET 7, A",    std::bind(setBitOfRegister, 0x1 << 7, &CpuRegisters::GetA, &CpuRegisters::SetA, false)};
#pragma endregion

    }
}