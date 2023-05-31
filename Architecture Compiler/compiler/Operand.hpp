#pragma once
#include <stdint.h>

class Operand
{
public:
	enum class Type : uint8_t
	{
		reg,
		modrm,
		imm
	};

	enum class Register : uint8_t
	{
		general,
		xmm,
		mm,
		bnd,
		st
	};

#ifdef INSTRUCTION_PACKING
#pragma pack(push, INSTRUCTION_PACKING)
#else
#pragma pack(push, 1)
#endif
	struct Package
	{
		struct
		{
			Type m_Type : 2;
			Register m_Register : 6;
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

private:

};