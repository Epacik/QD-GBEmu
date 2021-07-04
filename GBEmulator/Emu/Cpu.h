#pragma once
#ifndef CPU_H
#define CPU_H


#include "Bus.h"

namespace Emulator
{
	class Bus;

	class Cpu
	{
	public:
		Cpu();
		~Cpu();

		void Connect(Bus* bus);

	private:
		Bus* Bus = nullptr;

		void Write(uint16_t address, uint8_t data);
		uint8_t Read(uint16_t address);



	public:
		enum Flags {
			Zero = (1 << 7),
			Subtraction = (1 << 6), // BCD
			HalfCarry = (1 << 5), // BCD
			Carry = (1 << 4),
		};

		uint8_t Accumulator = 0x00;
		uint8_t FlagsState = 0x00;

		void SetFlag(Flags flag);

	};
}


#endif // !CPU_H