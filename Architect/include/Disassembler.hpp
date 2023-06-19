#pragma once
#include "archIL.hpp"
#include <vector>

class Disassembler
{
private:


public:
	std::vector<ILInstruction> Disassemble(const void* base, uint32_t size);

private:
	ILInstruction Disassemble(const uint8_t* instruction);

private:
	uint8_t CountToBit(uint32_t value);
};