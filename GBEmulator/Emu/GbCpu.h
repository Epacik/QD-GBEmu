#pragma once
#ifndef CPU_H
#define CPU_H


#include "Bus.h"
#include "CpuRegisters.h"

namespace Emulator
{
	class Bus;

	class GbCpu
	{
	public:
		GbCpu();
		~GbCpu();

		void Connect(Bus* bus);

	private:
		Bus* bus = nullptr;

		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address);



	public:
		enum Flags {
			Zero = (1 << 7),
			Subtraction = (1 << 6), // BCD
			HalfCarry = (1 << 5), // BCD
			Carry = (1 << 4),
		};

        CpuRegisters Registers;

		void SetFlag(Flags flag);
		void UnsetFlag(Flags flag);

		bool IsFlagSet(Flags flag);

	};
}


#endif // !CPU_H