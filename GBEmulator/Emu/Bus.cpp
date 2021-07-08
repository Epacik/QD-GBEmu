#include "Bus.h"
namespace Emulator {
    Bus::Bus() {
        // Reset values in devices connected to the Bus


        for (auto &item : VideoRam) {
            item = 0x00;
        }
        for (auto &item : ExternalRam) {
            item = 0x00;
        }
        for (auto &item : WorkRam) {
            item = 0x00;
        }
        for (auto &item : SpriteAttributeTable) {
            item = 0x00;
        }

        for (auto &item : IORegisters) {
            item = 0x00;
        }
        for (auto &item : HighRam) {
            item = 0x00;
        }

        Cpu = std::make_unique<GbCpu>();
        Cpu->Connect(this);
    }

    Bus::~Bus() {

    }

    void Bus::Write(uint16_t address, uint8_t data) {
        if (address >= 0x0000 && address <= 0x7FFF && Cartridge != nullptr) {
            Cartridge->Write(address, data);
        }
        else if (address >= 0x8000 && address <= 0x9FFF) {
            VideoRam[address - 0x8000] = data;
        }
        else if (address >= 0xA000 && address <= 0xBFFF) {
            Cartridge->Write(address, data);
            //ExternalRam[address - 0xA000] = data;
        }
        else if (address >= 0xC000 && address <= 0xDFFF) {
            WorkRam[address - 0xC000] = data;
        }
        else if (address >= 0xE000 && address <= 0xFDFF) {
            WorkRam[address - 0xE000] = data;
        }

        else if (address >= 0xFE00 && address <= 0xFE9F) {
            SpriteAttributeTable[address - 0xFE00] = data;
        }

        else if (address >= 0xFF00 && address <= 0xFF7F) {
            IORegisters[address - 0xFF00] = data;
        }

        else if (address >= 0xFF80 && address <= 0xFFFE) {
            HighRam[address - 0xFF80] = data;
        }

        else if(address == 0xFFFF){
            InterruptEnableRegister = data;
        }
    }

    uint8_t Bus::Read(uint16_t address, bool readonly) {
        if(address >= 0x0000 && address <= 0x00FF && !BootCompleted){
            return BootROM[address];
        }
        else if (address >= 0x0000 && address <= 0x7FFF) {
            return Cartridge != nullptr ? Cartridge->Read(address) : 0xFF;
        }
        else
        if (address >= 0x8000 && address <= 0x9FFF) {
            return VideoRam[address - 0x8000];
        }
        else if (address >= 0xA000 && address <= 0xBFFF) {
            return Cartridge != nullptr ? Cartridge->Read(address) : 0xFF;
            //return ExternalRam[address - 0xA000];
        }
        else if (address >= 0xC000 && address <= 0xDFFF) {
            return WorkRam[address - 0xC000];
        }
        else if (address >= 0xE000 && address <= 0xFDFF) {
            return WorkRam[address - 0xE000];
        }

        else if (address >= 0xFE00 && address <= 0xFE9F) {
            return SpriteAttributeTable[address - 0xFE00];
        }

        else if (address >= 0xFF00 && address <= 0xFF7F) {
            return IORegisters[address - 0xFF00];
        }

        else if (address >= 0xFF80 && address <= 0xFFFE) {
            return HighRam[address - 0xFF80];
        }

        else if(address == 0xFFFF){
            return InterruptEnableRegister;
        }

        return 0x00;
    }
}