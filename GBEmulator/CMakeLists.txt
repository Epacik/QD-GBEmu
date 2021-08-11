# CMakeList.txt: projekt CMake dla elementu GBEmulator, dołącz źródło i zdefiniuj
# tutaj logikę specyficzną dla projektu.
#
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


# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (GBEmulator WIN32 "Emu/GbCpu.h" "Emu/GbCpu.cpp" "Emu/GbBus.cpp" "Emu/GbBus.h" Emu/MemoryBankControllers/MemoryBankControllerBase.cpp Emu/MemoryBankControllers/MemoryBankControllerBase.h Emu/Devices/GbCartridge.h Application.cpp Application.h Windows/MainWindow.cpp Windows/MainWindow.h global.h Windows/Registers.cpp Windows/Registers.h Emu/CpuRegisters.cpp Emu/CpuRegisters.h Tools.cpp Tools.h Definitions/EventCallback.cpp Definitions/EventCallback.h Windows/Windows.h Emu/GbClock.cpp Emu/GbClock.h Other/Event.cpp Other/Event.h Emu/Instruction.h Emu/GbCpuOpcodes.cpp Windows/Memory.cpp Windows/Memory.h Emu/GbTimers.cpp Emu/GbTimers.h "Emu/BusDevice.h" Emu/GbJoypad.cpp Emu/GbJoypad.h Emu/Devices/GbCartridge.cpp Emu/Interrupts.h)
#add_executable (GBEmulator WIN32_EXECUTABLE "Emu/GbCpu.h" "Emu/GbCpu.cpp" "Emu/GbBus.cpp" "Emu/GbBus.h" Emu/MemoryBankControllers/MemoryBankControllerBase.cpp Emu/MemoryBankControllers/MemoryBankControllerBase.h Emu/Devices/GbCartridge.h Application.cpp Application.h Windows/MainWindow.cpp Windows/MainWindow.h global.h Windows/Registers.cpp Windows/Registers.h Emu/CpuRegisters.cpp Emu/CpuRegisters.h Tools.cpp Tools.h Definitions/EventCallback.cpp Definitions/EventCallback.h Windows/Windows.h Emu/GbClock.cpp Emu/GbClock.h Other/Event.cpp Other/Event.h Emu/Instruction.h Emu/GbCpuOpcodes.cpp Windows/Memory.cpp Windows/Memory.h Emu/GbTimers.cpp Emu/GbTimers.h)

# TODO: Dodaj testy i zainstaluj elementy docelowe w razie potrzeby.
target_link_libraries(GBEmulator ${wxWidgets_LIBRARIES})
