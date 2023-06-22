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
		rel
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

		struct
		{
			uint8_t m_Size8 : 1;
			uint8_t m_Size16 : 1;
			uint8_t m_Size32 : 1;
			uint8_t m_Size64 : 1;
			uint8_t m_Size128 : 1;
			uint8_t m_Size256 : 1;
			uint8_t m_Size512 : 1;
		};
	};
#pragma pack(pop)

public:
	Operand() = default;
	Operand(const DescriptorOperand& descriptor);

public:
	const Package& GetPackage() const;

private:
	Package m_Package = {};
};