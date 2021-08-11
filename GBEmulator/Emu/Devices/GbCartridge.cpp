#include <stdexcept>
#include "GbCartridge.h"
#include <format>
#include <string>
//
// Created by epat on 08.08.2021.
//

uint8_t Emulator::Cartridges::GbCartridgeHeader::CalculateHeaderChecksum()
{
    return uint8_t();
}

Emulator::Cartridges::GbCartridgeHeader::GbCartridgeHeader(
    std::array<uint8_t, 3> startVector,
    std::array<uint8_t, 0x2F> logo,
    std::array<uint8_t, 0xF> title,
    std::array<uint8_t, 2> newLicenseeCode,
    uint8_t sgbFlag,
    GbCartridgeTypes type,
    GbCartridgeRomSizes romSize,
    GbCartridgeRamSizes ramSize,
    GbCartridgeDestinationCodes destination,
    uint8_t licenseeCode,
    uint8_t version,
    uint8_t headerChecksum,
    std::array<uint8_t, 2> cartridgeChecksum)
{
    StartVector = startVector;
    Logo = logo;
    GameTitle = title;
    NewLicenseeCode = newLicenseeCode;
    SGBFlag = sgbFlag;
    
    if(!ValidateCartridgeType(type))
        throw std::invalid_argument("Unknown cartridge type: " + std::to_string((int)type) );

    CartridgeType = type;

}

bool Emulator::Cartridges::GbCartridgeHeader::ValidateCartridgeType(Emulator::Cartridges::GbCartridgeTypes type)
{
    switch (type) {
    case 0x04:
    case 0x07:
    case 0x0A:
    case 0x0E:
        return false;
    }
    if ((type >= 0x14 && type <= 0x18) || (type >= 0x23 && type <= 0xFB))
        return false;

    return true;
}


