//
// Created by epat on 08.08.2021.
//

#include "GbJoypad.h"


uint8_t Emulator::GbJoypad::Read()
{
    uint8_t value = 0b11001111;
    if (selectedMap & 0b100000 == 0) {
        uint8_t down  = IsButtonPressed(GbJoypadButtons::Down)  ? 0 : 1 << 3;
        uint8_t up    = IsButtonPressed(GbJoypadButtons::Up)    ? 0 : 1 << 2;
        uint8_t left  = IsButtonPressed(GbJoypadButtons::Left)  ? 0 : 1 << 1;
        uint8_t right = IsButtonPressed(GbJoypadButtons::Right) ? 0 : 1;

        return 0b11010000 | down | up | left | right;
    }

    if (selectedMap & 0b10000 == 0) {
        uint8_t start  = IsButtonPressed(GbJoypadButtons::Start)  ? 0 : 1 << 3;
        uint8_t select = IsButtonPressed(GbJoypadButtons::Select) ? 0 : 1 << 2;
        uint8_t b      = IsButtonPressed(GbJoypadButtons::B)      ? 0 : 1 << 1;
        uint8_t a      = IsButtonPressed(GbJoypadButtons::A)      ? 0 : 1;

        return 0b11100000 | start | select | b | a;
    }
    
    return 0b11111111;
}

bool Emulator::GbJoypad::IsButtonPressed(GbJoypadButtons button)
{
    return button & ButtonsPressed > 0;
}

void Emulator::GbJoypad::Write(uint8_t value)
{
    selectedMap = value & 0b110000;
    if(ButtonsPressed > 0 && (selectedMap >> 4) != 0b11 )
        Bus->SetInterruptFlag(Interrupts::JoypadInterrupt);
}

void Emulator::GbJoypad::PressButtons(GbJoypadButtons buttons)
{
    ButtonsPressed = buttons;
}

void Emulator::GbJoypad::Connect(GbBus* bus)
{
    Bus = bus;
}
