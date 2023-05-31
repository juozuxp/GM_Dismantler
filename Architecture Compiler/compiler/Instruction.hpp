#pragma once
#include "Operand.hpp"

class Instruction
{
public:
#pragma pack(push, 1)
	struct Package
	{
		uint16_t m_Type;
		Operand::Package m_Operands[4];
	};
#pragma pack(pop)

};