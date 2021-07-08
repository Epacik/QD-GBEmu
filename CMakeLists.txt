# CMakeList.txt: plik projektu CMake najwyższego poziomu, wykonaj konfigurację globalną
# i uwzględnij podprojekty w tym miejscu.
#
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)


project ("GBEmu")

# Uwzględnij podprojekty.
add_subdirectory ("GBEmulator")
