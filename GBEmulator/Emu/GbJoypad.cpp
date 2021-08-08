//
// Created by epat on 08.08.2021.
//

#include "GbJoypad.h"

uint8_t Emulator::GbJoypad::Read()
{
    uint8_t value = 0b11001111;
    if (selectedMap & 0b100000 == 0) {
        uint8_t down  = GbJoypadButtons::Down  & ButtonsPressed > 0 ? 0 : 1 << 3;
        uint8_t up    = GbJoypadButtons::Up    & ButtonsPressed > 0 ? 0 : 1 << 2;
        uint8_t left  = GbJoypadButtons::Left  & ButtonsPressed > 0 ? 0 : 1 << 1;
        uint8_t right = GbJoypadButtons::Right & ButtonsPressed > 0 ? 0 : 1;

        return 0b11010000 | down | up | left | right;
    }

    if (selectedMap & 0b10000 == 0) {
        uint8_t start  = GbJoypadButtons::Start  & ButtonsPressed > 0 ? 0 : 1 << 3;
        uint8_t select = GbJoypadButtons::Select & ButtonsPressed > 0 ? 0 : 1 << 2;
        uint8_t      b = GbJoypadButtons::B      & ButtonsPressed > 0 ? 0 : 1 << 1;
        uint8_t      a = GbJoypadButtons::A      & ButtonsPressed > 0 ? 0 : 1;

        return 0b11100000 | start | select | b | a;
    }
    
    return 0b11111111;
}

void Emulator::GbJoypad::Write(uint8_t value)
{
    selectedMap = value & 0b110000;

}

void Emulator::GbJoypad::PressButtons(GbJoypadButtons buttons)
{
    ButtonsPressed = buttons;
    Bus->Cpu->SetInterruptFlag(GbCpu::Interrupts::JoypadInterrupt);
}
