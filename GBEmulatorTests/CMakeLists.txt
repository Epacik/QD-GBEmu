cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

set(wxWidgets_CONFIGURATION mswu)
if (WIN32)
    SET(WX_ROOT_DIR $ENV{wxWidgets})
    set(wxWidgets_ROOT_DIR ${WX_ROOT_DIR})
    set(wxWidgets_LIB_DIR ${WX_ROOT_DIR}\\lib\\vc142_x64_dll)
    SET(wxWidgets_INCLUDE_DIRS ${WX_ROOT_DIR}\\include\\wx ${WX_ROOT_DIR}\\include\\msvc\\wx)
    set(wxWidgets_wxrc_EXECUTABLE ${WX_ROOT_DIR}\\lib\\vc142_x64_dll\\wxrc.exe)
    set(wxWidgets_USE_FILE true)
    find_package(wxWidgets 3.1 REQUIRED )
endif (WIN32)
if(UNIX AND NOT APPLE)
    find_package(wxWidgets COMPONENTS core base REQUIRED)
endif(UNIX AND NOT APPLE)
include(${wxWidgets_USE_FILE})

include_directories( ../GBEmulator)
# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (GBEmulatorTests
        Tests.cpp
        ../GBEmulator/Tools.h ../GBEmulator/Tools.cpp
        ../GBEmulator/Emu/Instruction.h ../GBEmulator/Emu/GbCpu.h ../GBEmulator/Emu/GbCpu.cpp ../GBEmulator/Emu/GbBus.h ../GBEmulator/Emu/GbBus.cpp
        ../GBEmulator/Emu/GbClock.h ../GBEmulator/Emu/GbClock.cpp ../GBEmulator/Emu/CpuRegisters.h ../GBEmulator/Emu/CpuRegisters.cpp
        ../GBEmulator/Emu/Devices/GbCartridge.h ../GBEmulator/Emu/MemoryBankControllers/MemoryBankControllerBase.h ../GBEmulator/Emu/MemoryBankControllers/MemoryBankControllerBase.cpp
        Cpu/Opcodes/00_0F.h ../GBEmulator/Emu/GbCpuOpcodes.cpp Cpu/Opcodes/Jumps.h Cpu/Opcodes/Tools.h Cpu/Opcodes/Increments.h Cpu/Opcodes/Decrements.h Cpu/Opcodes/Loads.h Cpu/Opcodes/Rotations.h Cpu/Opcodes.h ../GBEmulator/Emu/GbJoypad.cpp ../GBEmulator/Emu/GbJoypad.h)

target_link_libraries(GBEmulatorTests ${wxWidgets_LIBRARIES})
