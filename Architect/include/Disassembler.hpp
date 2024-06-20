#pragma once
#include "ArchitectureIL.hpp"
#include "Utility.hpp"
#include <vector>

class Disassembler
{
public:
	void Disassemble(const void* base, uint32_t size, std::vector<ILInstruction>& instructions) const;

private:
	ILInstruction Disassemble(const uint8_t* instruction) const;

private:
	static constexpr uint8_t GetSegment(Prefix prefixes)
	{
		uint8_t segment = Utility::CountToBit(prefixes.m_Prefix & c_SegmentPrefixMask);
		if (segment == static_cast<uint8_t>(~0))
		{
			return IL_INVALID_REGISTER;
		}

		return segment - c_SegmentPrefixBase;
	}

private:
	static constexpr uint8_t c_SegmentPrefixBase = Utility::CountToBit(Prefix_CS);
	static constexpr uint32_t c_SegmentPrefixMask = (Prefix_FS << 1) - Prefix_CS;
};