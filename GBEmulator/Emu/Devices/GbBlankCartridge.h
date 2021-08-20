//
// Created by epat on 20.08.2021.
//

#ifndef GBEMU_GBBLANKCARTRIDGE_H
#define GBEMU_GBBLANKCARTRIDGE_H
#include "GbCartridge.h"

namespace Emulator::Cartridges {
    class GbBlankCartridge : public GbCartridgeBase {
    public:
        GbBlankCartridge() = default;
        GbBlankCartridge(std::unique_ptr<GbCartridgeHeader> header){}

        void Write(uint16_t address, uint8_t data) override {

        }

        uint8_t Read(uint16_t address) override {
            return 0xFF;
        }
    };


}
#endif //GBEMU_GBBLANKCARTRIDGE_H
