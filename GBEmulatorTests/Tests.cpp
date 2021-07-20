#define CATCH_CONFIG_MAIN // provides main(); this line is required in only one .cpp file
#include "ThirdParty/Catch/catch.hpp"

#include "Cpu/Opcodes.h"




TEST_CASE( "Cpu", "[GameboyTests]" ) {
    using namespace Cpu::Opcodes;

    SECTION("Jumps"){
        SECTION("JR NZ,i8"){
            ExecJrNz(0xCCCF, 0x20, false);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF + 0x0020 + 2));

            ExecJrNz(0xCCCF, 0xFF, false);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF - 1 + 2));

            ExecJrNz(0xCCCF, 0x20, true);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF + 2));
        }
        SECTION("JR NC,i8"){
            ExecJrNc(0xCCCF, 0x20, false);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF + 0x0020 + 2));

            ExecJrNc(0xCCCF, 0xFF, false);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF - 1 + 2));

            ExecJrNc(0xCCCF, 0x20, true);
            CHECK(Bus.Cpu->Registers.PC == (0xCCCF + 2));
        }
    }

    SECTION("NOP") {
        ExecuteNop(0xCCC8);
        CHECK(Bus.Cpu->Registers.PC == 0xCCC9);

        ExecuteNop(0xFFF0);
        CHECK(Bus.Cpu->Registers.PC == 0xFFF1);
    }

    SECTION("Load double registers"){
        SECTION("LD BC, D16"){
            ExecLdDoubleD16(0xFF80, 0x42, 0x43);
            CHECK( Bus.Read(0xFF81)           == 0x43);
            CHECK( Bus.Read(0xFF82)           == 0x42);
            CHECK( Bus.Cpu->Registers.GetBC() == 0x4243 );


            ExecLdDoubleD16(0xFF88, 0x44, 0x45);
            CHECK( Bus.Read(0xFF89)           == 0x45);
            CHECK( Bus.Read(0xFF8A)           == 0x44);
            CHECK( Bus.Cpu->Registers.GetBC() == 0x4445 );
        }

        SECTION("LD DE, D16"){
            ExecLdDoubleD16(0xFF80, 0x42, 0x43, "de");
            CHECK( Bus.Read(0xFF81)           == 0x43);
            CHECK( Bus.Read(0xFF82)           == 0x42);
            CHECK( Bus.Cpu->Registers.GetDE() == 0x4243 );


            ExecLdDoubleD16(0xFF88, 0x44, 0x45, "de");
            CHECK( Bus.Read(0xFF89)           == 0x45);
            CHECK( Bus.Read(0xFF8A)           == 0x44);
            CHECK( Bus.Cpu->Registers.GetDE() == 0x4445 );
        }

        SECTION("LD HL, D16"){
            ExecLdDoubleD16(0xFF80, 0x42, 0x43, "hl");
            CHECK( Bus.Read(0xFF81)           == 0x43);
            CHECK( Bus.Read(0xFF82)           == 0x42);
            CHECK( Bus.Cpu->Registers.GetHL() == 0x4243 );


            ExecLdDoubleD16(0xFF88, 0x44, 0x45, "hl");
            CHECK( Bus.Read(0xFF89)           == 0x45);
            CHECK( Bus.Read(0xFF8A)           == 0x44);
            CHECK( Bus.Cpu->Registers.GetHL() == 0x4445 );
        }

        SECTION("LD SP, D16"){
            ExecLdDoubleD16(0xFF80, 0x42, 0x43, "sp");
            CHECK( Bus.Read(0xFF81)           == 0x43);
            CHECK( Bus.Read(0xFF82)           == 0x42);
            CHECK( Bus.Cpu->Registers.GetSP() == 0x4243 );


            ExecLdDoubleD16(0xFF88, 0x44, 0x45, "sp");
            CHECK( Bus.Read(0xFF89)           == 0x45);
            CHECK( Bus.Read(0xFF8A)           == 0x44);
            CHECK( Bus.Cpu->Registers.GetSP() == 0x4445 );
        }
    }

    SECTION("Load data from address in double registers"){
        SECTION("LD (BC), A"){
            ExecLdRegAddrA(0xFF8B, 0b10101010, 0xFF8C);
            CHECK( Bus.Read(0xFF8C)      == 0b10101010 );
            CHECK( Bus.Cpu->Registers.PC == 0xFF8C );
        }

        SECTION("LD (DE), A"){
            ExecLdRegAddrA(0xFF8B, 0b10101010, 0xFF8C, "de");
            CHECK( Bus.Read(0xFF8C)      == 0b10101010 );
            CHECK( Bus.Cpu->Registers.PC == 0xFF8C );
        }

        SECTION("LD (HL+), A"){
            ExecLdRegAddrA(0xFF8B, 0b10101010, 0xFF8C, "hl+");
            CHECK( Bus.Read(0xFF8C)      == 0b10101010 );
            CHECK( Bus.Cpu->Registers.PC == 0xFF8C );
            CHECK( Bus.Cpu->Registers.GetHL() == 0xFF8D );
        }

        SECTION("LD (BC), A"){
            ExecLdRegAddrA(0xFF8B, 0b10101010, 0xFF8C, "hl-");
            CHECK( Bus.Read(0xFF8C)      == 0b10101010 );
            CHECK( Bus.Cpu->Registers.PC == 0xFF8C );
            CHECK( Bus.Cpu->Registers.GetHL() == 0xFF8B );
        }
    }

    SECTION("Increment double registers"){
        SECTION("INC BC"){
            ExecIncDoubleRegister(0xC000, 0x0013);
            CHECK( Bus.Cpu->Registers.GetBC() == 0x0014 );
            ExecIncDoubleRegister(0xCFE0, 0x0014);
            CHECK( Bus.Cpu->Registers.GetBC() == 0x0015 );
        }

        SECTION("INC DE"){
            ExecIncDoubleRegister(0xC000, 0x0013, "de");
            CHECK( Bus.Cpu->Registers.GetDE() == 0x0014 );
            ExecIncDoubleRegister(0xCFE0, 0x0014, "de");
            CHECK( Bus.Cpu->Registers.GetDE() == 0x0015 );
        }

        SECTION("INC HL"){
            ExecIncDoubleRegister(0xC000, 0x0013, "hl");
            CHECK( Bus.Cpu->Registers.GetHL() == 0x0014 );
            ExecIncDoubleRegister(0xCFE0, 0x0014, "hl");
            CHECK( Bus.Cpu->Registers.GetHL() == 0x0015 );
        }

        SECTION("INC SP"){
            ExecIncDoubleRegister(0xC000, 0x0013, "sp");
            CHECK( Bus.Cpu->Registers.GetSP() == 0x0014 );
            ExecIncDoubleRegister(0xCFE0, 0x0014, "sp");
            CHECK( Bus.Cpu->Registers.GetSP() == 0x0015 );
        }
    }

    SECTION("Increment registers"){
        SECTION("INC B") {
            using namespace Emulator;
            ExecIncRegister(0xC230, 0b00001111);
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetB() == 0b00010000);

            ExecIncRegister(0xC230, 0xFF);
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetB() == 0b00000000);
        }

        SECTION("INC D") {
            using namespace Emulator;
            ExecIncRegister(0xC230, 0b00001111, "d");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetD() == 0b00010000);

            ExecIncRegister(0xC230, 0xFF, "d");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetD() == 0b00000000);
        }

        SECTION("INC H") {
            using namespace Emulator;
            ExecIncRegister(0xC230, 0b00001111, "h");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetH() == 0b00010000);

            ExecIncRegister(0xC230, 0xFF, "h");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetH() == 0b00000000);
        }
    }

    SECTION("Inc Memory"){
        using namespace Emulator;
        ExecIncMemAtAt(0xC230, 0xCCCF, 0b00001111);
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
        CHECK(       Bus.Read(0xCCCF) == 0b00010000);
    }

    SECTION("Decrement register"){

        SECTION("DEC B") {
            using namespace Emulator;
            ExecDecRegister(0xC230, 0b00001111);
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetB() == 0b00001110);

            ExecDecRegister(0xC230, 0xFF);
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetB() == 0b11111110);

            ExecDecRegister(0xC230, 0b11110000);
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetB() == 0b11101111);

            ExecDecRegister(0xC230, 1);
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(Bus.Cpu->Registers.GetB() == 0b00000000);
        }

        SECTION("DEC D") {
            using namespace Emulator;
            ExecDecRegister(0xC230, 0b00001111, "d");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetD() == 0b00001110);

            ExecDecRegister(0xC230, 0xFF, "d");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetD() == 0b11111110);

            ExecDecRegister(0xC230, 0b11110000, "d");
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetD() == 0b11101111);

            ExecDecRegister(0xC230, 1, "d");
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(Bus.Cpu->Registers.GetD() == 0b00000000);
        }

        SECTION("DEC H") {
            using namespace Emulator;
            ExecDecRegister(0xC230, 0b00001111, "h");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetH() == 0b00001110);

            ExecDecRegister(0xC230, 0xFF, "h");
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetH() == 0b11111110);

            ExecDecRegister(0xC230, 0b11110000, "h");
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(       Bus.Cpu->Registers.GetH() == 0b11101111);

            ExecDecRegister(0xC230, 1, "h");
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
            CHECK(Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
            CHECK(Bus.Cpu->Registers.GetH() == 0b00000000);
        }
    }

    SECTION("dec Memory"){
        using namespace Emulator;
        ExecDecMemAtAt(0xC230, 0xCCCF, 0b00001111);
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
        CHECK(       Bus.Read(0xCCCF) == 0b00001110);

        ExecDecMemAtAt(0xC230, 0xCCCF, 0xFF);
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
        CHECK(       Bus.Read(0xCCCF) == 0b11111110);

        ExecDecMemAtAt(0xC230, 0xCCCF, 0b11110000);
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
        CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
        CHECK(       Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
        CHECK(       Bus.Read(0xCCCF) == 0b11101111);

        ExecDecMemAtAt(0xC230, 0xCCCF, 1);
        CHECK( Bus.Cpu->IsFlagSet(GbCpu::Flags::HalfCarry));
        CHECK( Bus.Cpu->IsFlagSet(GbCpu::Flags::Zero));
        CHECK( Bus.Cpu->IsFlagSet(GbCpu::Flags::Subtraction));
        CHECK( Bus.Read(0xCCCF) == 0b00000000);
    }

    SECTION("Load data"){
        SECTION("Load immediate to register"){
            SECTION("B"){
                ExecLoadImmDataToRegister(0xCCFC, 0x12 );
                CHECK( Bus.Cpu->Registers.GetB() == 0x12 );

                ExecLoadImmDataToRegister(0xCCFC, 0x69 );
                CHECK( Bus.Cpu->Registers.GetB() == 0x69 );

                ExecLoadImmDataToRegister(0xCCFC, 0x21 );
                CHECK( Bus.Cpu->Registers.GetB() == 0x21 );
            }

            SECTION("D"){
                ExecLoadImmDataToRegister(0xCCFC, 0x12, "D");
                CHECK( Bus.Cpu->Registers.GetD() == 0x12 );

                ExecLoadImmDataToRegister(0xCCFC, 0x69, "D");
                CHECK( Bus.Cpu->Registers.GetD() == 0x69 );

                ExecLoadImmDataToRegister(0xCCFC, 0x21, "D");
                CHECK( Bus.Cpu->Registers.GetD() == 0x21 );
            }

            SECTION("H"){
                ExecLoadImmDataToRegister(0xCCFC, 0x12, "H");
                CHECK( Bus.Cpu->Registers.GetH() == 0x12 );

                ExecLoadImmDataToRegister(0xCCFC, 0x69, "H");
                CHECK( Bus.Cpu->Registers.GetH() == 0x69 );

                ExecLoadImmDataToRegister(0xCCFC, 0x21, "H");
                CHECK( Bus.Cpu->Registers.GetH() == 0x21 );
            }
        }
    }

    SECTION("Rotation"){
        using namespace Emulator;
        SECTION("RLCA"){
            ExecRLCA(0xCFCF, 0b10101010 );
            CHECK( Bus.Cpu->Registers.GetA() == 0b01010101 );
            CHECK( Bus.Cpu->IsFlagSet(GbCpu::Flags::Carry) );

            ExecRLCA(0xCFCF, 0b01010101  );
            CHECK( Bus.Cpu->Registers.GetA() == 0b10101010 );
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Carry) );
        }

        SECTION("RLA"){
            ExecRLA(0xCFCF, 0b10101010 );
            CHECK( Bus.Cpu->Registers.GetA() == 0b01010100 );
            CHECK( Bus.Cpu->IsFlagSet(GbCpu::Flags::Carry) );

            Bus.Cpu->SetFlag(GbCpu::Flags::Carry);
            ExecRLA(0xCFCF, 0b01010101 );

            CHECK( Bus.Cpu->Registers.GetA() == 0b10101011 );
            CHECK_FALSE( Bus.Cpu->IsFlagSet(GbCpu::Flags::Carry) );
        }
    }

}

TEST_CASE("Bus", "[GameboyTests]"){

}