#include "GbBus.h"

#include <memory>

class Application;
namespace Emulator {
    GbBus::GbBus(bool stop) : GbBus() {
        Clock.Stop();
    }

    GbBus::GbBus() {

        InitializeMemory();

        ConnectDevices();

        SetOnClockCycle();
    }

    void GbBus::ConnectDevices()
    {
        Cpu = std::make_unique<GbCpu>();
        Cpu->Connect(this);

        Timers = std::make_unique<GbTimers>();
        Timers->Connect(this);

        Joypad = std::make_unique<GbJoypad>();

        Cartridge.reset(new Cartridges::GbBlankCartridge());
    }

    void GbBus::InitializeMemory()
    {
        // Reset values in devices connected to the GbBus
        for (auto& item : VideoRam) {
            item = 0x00;
        }
        /*for (auto &item : ExternalRam) {
        item = 0x00;
        }*/
        for (auto& item : WorkRam) {
            item = 0x00;
        }
        for (auto& item : SpriteAttributeTable) {
            item = 0x00;
        }

        for (auto& item : IORegisters) {
            item = 0x00;
        }
        for (auto& item : HighRam) {
            item = 0x00;
        }
    }

    void GbBus::SetOnClockCycle()
    {
        uint32_t clockNum = 0;

        Clock.OnClockCycle = std::make_shared<std::function<void()>>([this] {
            if (clockCycle % 4 == 0)
                Cpu->OnClockCycle();

            Timers->OnClockCycle();

            //App
            if (OnRefreshUI != nullptr) {
                (*OnRefreshUI)();
            }
            clockCycle++;
            });
    }




    GbBus::~GbBus() {

    }

    void GbBus::Write(uint16_t address, uint8_t data) {
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

        else if (address >= 0xFF04 && address <= 0xFF07){
            Timers->Write(address, data);
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

    uint8_t GbBus::Read(uint16_t address, bool readonly) {
        if(address >= 0x0000 && address <= 0x00FF && !BootCompleted){
            return BootROM[address];
        }
        else if (address >= 0x0000 && address <= 0x7FFF) {
            return Cartridge != nullptr ? Cartridge->Read(address) : 0x00;//0xFF;
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

        else if (address >= 0xFF04 && address <= 0xFF07){
            return Timers->Read(address);
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

    void GbBus::SetInterruptFlag(Emulator::Interrupts interrupt)
    {
        //Cpu->SetInterruptFlag(interrupt);
    }

    void GbBus::SetOnRefreshUI(std::shared_ptr<std::function<void()>> func) {
        OnRefreshUI = func;
    }

    void GbBus::HangTheWholeEmulator() {

    }
}