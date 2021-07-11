# CMakeList.txt: projekt CMake dla elementu GBEmulator, dołącz źródło i zdefiniuj
# tutaj logikę specyficzną dla projektu.
#
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})


# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (GBEmulator "Emu/GbCpu.h" "Emu/GbCpu.cpp" "Emu/Bus.cpp" "Emu/Bus.h" Emu/MemoryBankControllers/MemoryBankControllerBase.cpp Emu/MemoryBankControllers/MemoryBankControllerBase.h Emu/Devices/GbCartridge.h Application.cpp Application.h Windows/MainWindow.cpp Windows/MainWindow.h global.h Windows/Registers.cpp Windows/Registers.h Emu/CpuRegisters.cpp Emu/CpuRegisters.h Tools.cpp Tools.h Definitions/EventCallback.cpp Definitions/EventCallback.h)
# TODO: Dodaj testy i zainstaluj elementy docelowe w razie potrzeby.
target_link_libraries(GBEmulator ${wxWidgets_LIBRARIES})
