#pragma once
#include "ArchitectureIL.hpp"
#include <vector>

class Disassembler_Utility
{
protected:
	static constexpr uint8_t CountToBit(uint32_t value)
	{
		if (value == 0)
		{
			return ~0;
		}

		for (uint8_t i = 0;; i++, value >>= 1)
		{
			if (value & 1)
			{
				return i;
			}
		}

		return ~0; // impossible value return, here to shut the compiler up
	}
};

class Disassembler : Disassembler_Utility
{
public:
	std::vector<ILInstruction> Disassemble(const void* base, uint32_t size);

private:
	ILInstruction Disassemble(const uint8_t* instruction);

private:
	static constexpr uint8_t GetSegment(Prefix prefixes)
	{
		uint8_t segment = CountToBit(prefixes.m_Prefix & c_SegmentPrefixMask);
		if (segment == static_cast<uint8_t>(~0))
		{
			return IL_INVALID_REGISTER;
		}

		return segment - c_SegmentPrefixBase;
	}

private:
	static constexpr uint8_t c_SegmentPrefixBase = CountToBit(Prefix_CS);
	static constexpr uint32_t c_SegmentPrefixMask = (Prefix_FS << 1) - Prefix_CS;
};