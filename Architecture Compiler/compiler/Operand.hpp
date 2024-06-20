#pragma once
#include <stdint.h>
#include "parser/DescriptorOperand.hpp"

class Operand
{
public:
	enum class Type : uint8_t
	{
		none,
		reg,
		modrm,
		imm,
		rel,
		moffs
	};

#pragma pack(push, 1)
	struct Package
	{
		struct
		{
			Type m_Type : 3;
			DescriptorOperand::Register m_Register : 5;
		};

		struct
		{
			uint8_t m_Rex : 1;
			uint8_t m_Constant : 1;
			uint8_t m_RegisterIndex : 4;
		};

		DescriptorOperand::OperandSize m_Reg;
		DescriptorOperand::OperandSize m_Mem;
	};
#pragma pack(pop)

public:
	Operand() = default;
	Operand(const DescriptorOperand& descriptor, bool regOnly);

public:
	const Package& GetPackage() const;

private:
	Package m_Package = {};

private:
	friend class Instruction;
};