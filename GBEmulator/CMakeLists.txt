# CMakeList.txt: projekt CMake dla elementu GBEmulator, dołącz źródło i zdefiniuj
# tutaj logikę specyficzną dla projektu.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)


# Dodaj źródło do pliku wykonywalnego tego projektu.
add_executable (GBEmulator "GBEmulator.cpp" "GBEmulator.h" "Emu/Cpu.h" "Emu/Cpu.cpp" "Emu/Bus.cpp" "Emu/Bus.h" )
# TODO: Dodaj testy i zainstaluj elementy docelowe w razie potrzeby.
