//
// Created by epat on 06.07.2021.
//

#ifndef GBEMU_GBCARTRIDGE_H
#define GBEMU_GBCARTRIDGE_H

#include <cstdint>
#include <memory>
#include <array>
namespace Emulator::Cartridges {

    /// <summary>
    /// Typy cartridg�w
    /// </summary>
    enum class GbCartridgeTypes : uint8_t {
        RomOnly = 0x00,
        MBC1 = 0x01,
        MBC1Ram = 0x02,
        MBC1RamBattery = 0x03,

        MBC2 = 0x05,
        MBC2RamBattery = 0x06,

        RomRam = 0x08,
        RomRamBattery = 0x09,

        MMM01 = 0x0B,
        MMM01Ram = 0x0C,
        MMM0RamBattery = 0x0D,

        MBC3TimerBattery = 0x0F,
        MBC3RamTimerBattery = 0x10,
        MBC3 = 0x11,
        MBC3Ram = 0x12,
        MBC3RamBattery = 0x13,

        MBC5 = 0x19,
        MBC5Ram = 0x1A,
        MBC5RamBattery = 0x1B,
        MBC5Rumble = 0x1C,
        MBC5RamRumble = 0x1D,
        MBC5RamBatteryRumble = 0x1E,

        MBC6RamBattery = 0x20,

        MBC7RamBatteryAccelerometer = 0x22,

        PocketCamera = 0xFC,
        BandaiTama5 = 0xFD,
        HuC3 = 0xFE,
        HuC1RamBattery = 0xFF,
    };
    enum class GbCartridgeRomSizes: uint8_t{
        _32K2Banks = 0x00,
        _64K4Banks = 0x01,
        _128K8Banks = 0x02,
        _256K16Banks = 0x03,
        _512K32Banks = 0x04,
        _1M64Banks = 0x05,
        _2M128Banks = 0x06,
        _4M256Banks = 0x07,
        _8M512Banks = 0x08,
    };
    enum class GbCartridgeRamSizes : uint8_t {
        None = 0x00,
        _2K1Bank = 0x01,
        _8K1Bank = 0x02,
        _32K4Banks = 0x03,
        _128K16Banks = 0x04,
        _64K8Banks = 0x05,
    };
    enum class GbCartridgeDestinationCodes : uint8_t {
        Japan = 0x00,
        RestOfTheWorld = 0x01,
    };

    struct GbCartridgeHeader {
    public:
        std::array<uint8_t, 3>      StartVector;       // 0x100 -> 0x103
        std::array<uint8_t, 0x2F>   Logo;              // 0x104 -> 0x133
        std::array<uint8_t, 0xF>    GameTitle;         // 0x134 -> 0x143
        std::array<uint8_t, 2>      NewLicenseeCode;   // 0x144 -> 0x145
        uint8_t                     SGBFlag = 0x00;    // 0x146
        GbCartridgeTypes            CartridgeType;     // 0x147
        GbCartridgeRomSizes         RomSize;           // 0x148
        GbCartridgeRamSizes         RamSize;           // 0x149
        GbCartridgeDestinationCodes DestinationCode;   // 0x14A
        uint8_t                     OldLicenseeCode;   // 0x14B
        uint8_t                     RomVersion;        // 0x14C
        uint8_t                     HeaderChecksum;    // 0x14D
        std::array<uint8_t, 2>      CartridgeChecksum; // 0x14E -> 0x14F

        uint8_t CalculateHeaderChecksum();

        GbCartridgeHeader(
            std::array<uint8_t, 3>      startVector,
            std::array<uint8_t, 0x2F>   logo,
            std::array<uint8_t, 0xF>    title,
            std::array<uint8_t, 2>      newLicenseeCode,
            uint8_t                     sgbFlag,
            GbCartridgeTypes            type,
            GbCartridgeRomSizes         romSize,
            GbCartridgeRamSizes         ramSize,
            GbCartridgeDestinationCodes destination,
            uint8_t                     licenseeCode,
            uint8_t                     version,
            uint8_t                     headerChecksum,
            std::array<uint8_t, 2>      cartridgeChecksum);

    private:
        bool ValidateCartridgeType(Emulator::Cartridges::GbCartridgeTypes type);
    };

    class GbCartridgeBase {

    public:
        virtual void Write(uint16_t address, uint8_t data) = 0;
        virtual uint8_t Read(uint16_t address) = 0;
        std::unique_ptr<GbCartridgeHeader> Header;

        GbCartridgeBase(){}
        GbCartridgeBase(std::unique_ptr<GbCartridgeHeader> header);
        ~GbCartridgeBase() = default;

    };
    
}


#endif //GBEMU_GBCARTRIDGE_H
