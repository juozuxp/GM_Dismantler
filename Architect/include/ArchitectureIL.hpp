#pragma once
#include "x86_x64.hpp"

#pragma pack(push, 1)
enum ILOperandType : uint8_t
{
	ILOperandType_None,
	ILOperandType_Memory,
	ILOperandType_Register,
	ILOperandType_Value,
	ILOperandType_ValueRelative,
	ILOperandType_MemoryRelative,
	ILOperandType_MemoryAbsolute
};
#pragma pack(pop)

#pragma pack(push, 1)
enum ILMemoryScaler : uint8_t
{
	ILMemoryScaler_1,
	ILMemoryScaler_2,
	ILMemoryScaler_4,
	ILMemoryScaler_8
};
#pragma pack(pop)

#pragma pack(push, 1)
enum ILOperandScale : uint8_t
{
	ILOperandScale_Undefined,
	ILOperandScale_8,
	ILOperandScale_16,
	ILOperandScale_32,
	ILOperandScale_64,
	ILOperandScale_128,
	ILOperandScale_256,
	ILOperandScale_512
};
#pragma pack(pop)

#define IL_INVALID_REGISTER static_cast<uint8_t>((1 << 7) - 1)

#pragma pack(push, 1)
struct ILOperand 
{
	struct
	{
		ILOperandType m_Type : 4;
		ILOperandScale m_Scale : 4;
	};
	union
	{
		struct
		{
			ILMemoryScaler m_Scale;

			uint8_t m_Base; // register index, IL_INVALID_REGISTER - reserved for none
			uint8_t m_Index; // register index, IL_INVALID_REGISTER - reserved for none

			int32_t m_Offset;
		} m_Memory; // ILOperandType_Memory
		struct
		{
			struct
			{
				uint8_t m_Base : 7; // register index, IL_INVALID_REGISTER - reserved for none
				uint8_t m_BaseHigh : 1; // is register high
			};

			RegisterType m_Type;
		} m_Register; // ILOperandType_Register

		uint64_t m_Value; // if ILOperandType_MemoryAbsolute specified, [m_Value], if ILOperandType_MemoryRelative specified, [rip + m_Value], if ILOperandType_ValueRelative rip + m_Value
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ILInstruction
{
	InstructionType m_Type = Instruction_invalid;
	ILOperand m_Operands[4];
};
#pragma pack(pop)