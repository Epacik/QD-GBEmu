//
// Created by epat on 06.07.2021.
//

#ifndef GBEMU_GBCARTRIDGE_H
#define GBEMU_GBCARTRIDGE_H

#include <cstdint>

class GbCartridgeBase {
public:
    virtual void WriteRom(uint16_t address, uint8_t data) = 0;
    virtual uint8_t ReadRom(uint16_t address) = 0;

    virtual void WriteRam(uint16_t address, uint8_t data) = 0;
    virtual uint8_t ReadRam(uint16_t address) = 0;
};


#endif //GBEMU_GBCARTRIDGE_H
