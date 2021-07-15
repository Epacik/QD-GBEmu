#define CATCH_CONFIG_MAIN // provides main(); this line is required in only one .cpp file
#include "ThirdParty/Catch/catch.hpp"

#include "Cpu/Opcodes/Nop.h"
#include "Cpu/Opcodes/LD_BC_d16.h"


TEST_CASE( "Cpu", "[OpCodesTests]" ) {
    SECTION("NOP") {
        using namespace Cpu::Opcodes;
        REQUIRE(TestNop(3) == 4);
        REQUIRE(TestNop(10) == 11);
    }
    SECTION("LD BC, D16"){
        using namespace Cpu::Opcodes;
        REQUIRE(TestLdBcD16(0xFF80, 0x42, 0x43) == 0x4243 );
    }
}