cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories( ../GBEmulator)
# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (GBEmulatorTests
        Tests.cpp
        ../GBEmulator/Tools.h ../GBEmulator/Tools.cpp
        ../GBEmulator/Emu/Instruction.h ../GBEmulator/Emu/GbCpu.h ../GBEmulator/Emu/GbCpu.cpp ../GBEmulator/Emu/GbBus.h ../GBEmulator/Emu/GbBus.cpp
        ../GBEmulator/Emu/GbClock.h ../GBEmulator/Emu/GbClock.cpp ../GBEmulator/Emu/CpuRegisters.h ../GBEmulator/Emu/CpuRegisters.cpp
        ../GBEmulator/Emu/Devices/GbCartridge.h ../GBEmulator/Emu/MemoryBankControllers/MemoryBankControllerBase.h ../GBEmulator/Emu/MemoryBankControllers/MemoryBankControllerBase.cpp
        Cpu/Opcodes/00_0F.h ../GBEmulator/Emu/GbCpuOpcodes.cpp Cpu/Opcodes/Jumps.h Cpu/Opcodes/Tools.h)

target_link_libraries(GBEmulatorTests ${wxWidgets_LIBRARIES})
